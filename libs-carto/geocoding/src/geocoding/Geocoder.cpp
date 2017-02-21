#include "Geocoder.h"
#include "FeatureReader.h"
#include "ProjUtils.h"
#include "AddressInterpolator.h"

#include <functional>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sqlite3pp.h>

namespace {
    std::string escapeSQLValue(const std::string& val) {
        return boost::replace_all_copy(val, "'", "''");
    }
}

namespace carto { namespace geocoding {
    bool Geocoder::getAutocomplete() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _autocomplete;
    }

    void Geocoder::setAutocomplete(bool autocomplete) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
         _autocomplete = autocomplete;
    }

    std::string Geocoder::getLanguage() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _language;
    }

    void Geocoder::setLanguage(const std::string& language) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _language = language;
        _addressCache.clear();
        _nameQueryCache.clear();
        _nameMatchCache.clear();
    }

    bool Geocoder::isFilterEnabled(Address::Type type) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return std::find(_enabledFilters.begin(), _enabledFilters.end(), type) != _enabledFilters.end();
    }

    void Geocoder::setFilterEnabled(Address::Type type, bool enabled) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        auto it = std::find(_enabledFilters.begin(), _enabledFilters.end(), type);
        if (enabled && it == _enabledFilters.end()) {
            _enabledFilters.push_back(type);
        }
        else if (!enabled && it != _enabledFilters.end()) {
            _enabledFilters.erase(it);
        }
    }

    std::vector<std::pair<Address, float>> Geocoder::findAddresses(const std::string& queryString, const Options& options) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        std::string safeQueryString = boost::replace_all_copy(boost::replace_all_copy(queryString, "%", ""), "_", "");
        boost::trim(safeQueryString);

        bool autocomplete = _autocomplete && safeQueryString.size() >= MIN_AUTOCOMPLETE_SIZE;
        if (!safeQueryString.empty() && TokenListType::isSeparator(safeQueryString.back())) {
            autocomplete = false;
        }
        if (autocomplete) {
            // Tricky, do not use autocomplete if the query ends with space. Otherwise append % sign that has special meaning
            safeQueryString += (boost::trim_right_copy(queryString) != queryString ? " " : "%");
        }

        Query query;
        query.options = options;
        query.tokenList = TokenListType::build(safeQueryString);

        // Classify tokens
        classifyTokens(query.tokenList);

        // Find candidates by matching tokens
        std::vector<Query> candidates;
        matchTokens(query, candidates);

        // Group by unmatched tokens
        std::map<int, std::vector<Query>> candidateGroupMap;
        for (const Query& query : candidates) {
            candidateGroupMap[query.unmatchedTokens()].push_back(query);
        }
        
        // Resolve addresses in groups
        std::vector<Result> results;
        for (auto it = candidateGroupMap.begin(); it != candidateGroupMap.end(); it++) {
            if (!results.empty()) {
                break;
            }

            for (Query& query : it->second) {
                if (optimizeQuery(query)) {
                    resolveAddress(query, results);
                }
            }
        }

        // Create address data from the results by merging consecutive results, if possible
        std::vector<std::pair<Address, float>> addresses;
        for (const Result& result : results) {
            if (addresses.size() >= MAX_RESULTS) {
                break;
            }

            Address address;
            if (!_addressCache.read(result.encodedId, address)) {
                address.loadFromDB(_db, result.encodedId, _language, [this](const cglib::vec2<double>& pos) {
                    return _origin + pos;
                });
                _addressCache.put(result.encodedId, address);
            }

            if (!addresses.empty() && result.ranking.rank() == addresses.back().second) {
                if (addresses.back().first.merge(address)) {
                    continue;
                }
            }
            addresses.emplace_back(address, result.ranking.rank());
        }
        return addresses;
    }

    void Geocoder::classifyTokens(TokenListType& tokenList) const {
        for (int i = 0; i < tokenList.size(); i++) {
            std::string tokenValue = tokenList.tokens(TokenListType::Span{ i, 1 }).front();

            // Do token translation, actual tokens are normalized relative to real names using translation table
            unistring token;
            for (unichar_t c : toUniString(tokenValue)) {
                auto it = _translationTable.find(c);
                if (it != _translationTable.end()) {
                    token += it->second;
                }
                else {
                    token.append(1, c);
                }
            }

            // Build token info list for the token
            if (!token.empty()) {
                for (int pass = 0; pass < 2; pass++) {
                    std::string sql = "SELECT id, idf, classes FROM tokens WHERE ";
                    if (pass == 1 && token.size() >= 2) {
                        sql += "token LIKE '" + escapeSQLValue(toUtf8String(token.substr(0, 2))) + "%' COLLATE NOCASE ORDER BY idf ASC LIMIT 10";
                    }
                    else if (!token.empty() && token.back() == '%') {
                        sql += "token LIKE '" + escapeSQLValue(toUtf8String(token)) + "' COLLATE NOCASE ORDER BY idf ASC LIMIT 10";
                    }
                    else {
                        sql += "token='" + escapeSQLValue(toUtf8String(token)) + "' COLLATE NOCASE";
                    }
                    sqlite3pp::query sqlQuery(_db, sql.c_str());

                    std::vector<Token> tokens;
                    for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                        Token token;
                        token.id = qit->get<std::uint64_t>(0);
                        token.idf = static_cast<float>(qit->get<double>(1));
                        tokens.push_back(token);
                        for (int n = 0; n < 32; n++) {
                            if (qit->get<int>(2) & (1 << n)) {
                                TokenType tokenType = static_cast<TokenType>(n);
                                tokenList.markValidType(i, tokenType);
                            }
                        }
                    }
                    tokenList.setTag(i, tokens);
                    if (!tokens.empty()) {
                        break;
                    }
                }
            }
        }
    }

    void Geocoder::matchTokens(const Query& query, std::vector<Query>& candidates) const {
        std::vector<TokenType> validTypes = { TokenType::COUNTRY, TokenType::REGION, TokenType::COUNTY, TokenType::LOCALITY, TokenType::NEIGHBOURHOOD, TokenType::STREET, TokenType::POSTCODE, TokenType::NAME };
        if (!query.streets.empty()) {
            validTypes.push_back(TokenType::HOUSENUMBER);
        }
        
        TokenType type = TokenType::NONE;
        std::vector<TokenListType::Span> spans = query.tokenList.enumerate(validTypes, type);
        if (type == TokenType::NONE) {
            candidates.push_back(query);
            return;
        }
        
        for (const TokenListType::Span& span : spans) {
            Query subQuery = query;
            bool valid = true;
            switch (type) {
            case TokenType::COUNTRY:
                valid = bindQueryNameField(subQuery, type, &Query::countries, span);
                break;
            case TokenType::REGION:
                valid = bindQueryNameField(subQuery, type, &Query::regions, span);
                break;
            case TokenType::COUNTY:
                valid = bindQueryNameField(subQuery, type, &Query::counties, span);
                break;
            case TokenType::LOCALITY:
                valid = bindQueryNameField(subQuery, type, &Query::localities, span);
                break;
            case TokenType::NEIGHBOURHOOD:
                valid = bindQueryNameField(subQuery, type, &Query::neighbourhoods, span);
                break;
            case TokenType::STREET:
                valid = bindQueryNameField(subQuery, type, &Query::streets, span);
                break;
            case TokenType::POSTCODE:
                valid = bindQueryNameField(subQuery, type, &Query::postcodes, span);
                break;
            case TokenType::NAME:
                valid = bindQueryNameField(subQuery, type, &Query::names, span);
                break;
            case TokenType::HOUSENUMBER:
                valid = bindQueryStringField(subQuery, type, &Query::houseNumber, span);
                break;
            }
            if (valid) {
                matchTokens(subQuery, candidates);
            }
        }
    }

    void Geocoder::resolveAddress(const Query& query, std::vector<Result>& results) const {
        std::vector<std::string> sqlFilters = buildQueryFilters(query);
        if (sqlFilters.empty()) {
            return;
        }

        std::string sql = "SELECT id, housenumbers, features, country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id, name_id FROM entities WHERE ";
        for (std::size_t i = 0; i < sqlFilters.size(); i++) {
            sql += (i > 0 ? " AND (" : "(") + sqlFilters[i] + ")";
        }

        auto mercatorConverter = [this](const cglib::vec2<double>& pos) {
            return wgs84ToWebMercator(_origin + pos);
        };

        sqlite3pp::query sqlQuery(_db, sql.c_str());
        for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
            unsigned int entityId = qit->get<unsigned int>(0);
            unsigned int elementIndex = 0;

            // Match house number
            auto houseNumbers = qit->get<const char*>(1);
            if (!query.houseNumber.empty()) {
                if (houseNumbers) {
                    AddressInterpolator interpolator(houseNumbers);
                    elementIndex = interpolator.findAddress(query.houseNumber) + 1; // if not found, interpolator returns -1
                    if (!elementIndex) {
                        continue;
                    }
                }
                else {
                    continue;
                }
            }

            // Do field match ranking and population ranking
            float matchRank = 1.0f;
            matchRank *= calculateMatchRank(TokenType::COUNTRY,       query.countries,       qit->get<std::uint64_t>(3), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::REGION,        query.regions,        qit->get<std::uint64_t>(4), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::COUNTY,        query.counties,        qit->get<std::uint64_t>(5), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::LOCALITY,      query.localities,      qit->get<std::uint64_t>(6), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::NEIGHBOURHOOD, query.neighbourhoods, qit->get<std::uint64_t>(7), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::STREET,        query.streets,        qit->get<std::uint64_t>(8), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::POSTCODE,      query.postcodes,      qit->get<std::uint64_t>(9), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::NAME,          query.names,          qit->get<std::uint64_t>(10), query.tokenList);

            float populationRank = 1.0f;
            populationRank *= calculatePopulationRank(TokenType::COUNTRY,       qit->get<std::uint64_t>(3));
            populationRank *= calculatePopulationRank(TokenType::REGION,        qit->get<std::uint64_t>(4));
            populationRank *= calculatePopulationRank(TokenType::COUNTY,        qit->get<std::uint64_t>(5));
            populationRank *= calculatePopulationRank(TokenType::LOCALITY,      qit->get<std::uint64_t>(6));
            populationRank *= calculatePopulationRank(TokenType::NEIGHBOURHOOD, qit->get<std::uint64_t>(7));
            populationRank *= calculatePopulationRank(TokenType::STREET,        qit->get<std::uint64_t>(8));
            populationRank *= (qit->get<std::uint64_t>(10) != 0 ? POI_POPULATION_PENALTY : 1.0f);

            // Do location based ranking
            float locationRank = 1.0f;
            if (query.options.location) {
                EncodingStream stream(qit->get<const void*>(2), qit->column_bytes(2));
                FeatureReader reader(stream, mercatorConverter);

                std::vector<Feature> features;
                if (elementIndex) {
                    AddressInterpolator interpolator(houseNumbers);
                    features = interpolator.enumerateAddresses(reader).at(elementIndex - 1).second;
                }
                else {
                    features = reader.readFeatureCollection();
                }

                float minDist = query.options.locationRadius;
                for (const Feature& feature : features) {
                    if (std::shared_ptr<Geometry> geometry = feature.getGeometry()) {
                        cglib::vec2<double> mercatorMeters = webMercatorMeters(*query.options.location);
                        cglib::vec2<double> mercatorLocation = wgs84ToWebMercator(*query.options.location);
                        cglib::vec2<double> point = geometry->calculateNearestPoint(mercatorLocation);
                        cglib::vec2<double> diff = point - mercatorLocation;
                        float dist = static_cast<float>(cglib::length(cglib::vec2<double>(diff(0) * mercatorMeters(0), diff(1) * mercatorMeters(1))));
                        minDist = std::min(minDist, dist);
                    }
                }

                float c = -std::log(MIN_LOCATION_RANK) / query.options.locationRadius;
                locationRank *= std::exp(-minDist * c);
            }

            // Build the result
            Result result;
            result.encodedId = (static_cast<std::uint64_t>(elementIndex) << 32) | entityId;
            result.unmatchedTokens = query.unmatchedTokens();
            result.ranking.matchRank = matchRank * std::pow(UNMATCHED_FIELD_PENALTY, static_cast<float>(result.unmatchedTokens));
            result.ranking.populationRank = populationRank;
            result.ranking.locationRank = locationRank;

            // Check if the same result is already stored
            auto resultIt = std::find_if(results.begin(), results.end(), [&result](const Result& result2) {
                return result.encodedId == result2.encodedId;
            });
            if (resultIt != results.end()) {
                if (resultIt->ranking.rank() >= result.ranking.rank()) {
                    continue; // if we have stored the row with better ranking, ignore current
                }
                results.erase(resultIt); // erase the old match, as the new match is better
            }

            // Find position for the result
            resultIt = std::upper_bound(results.begin(), results.end(), result, [](const Result& result1, const Result& result2) {
                return result1.ranking.rank() > result2.ranking.rank();
            });
            results.insert(resultIt, result);
            
            // Drop results that have too low rankings
            while (!results.empty()) {
                if (results.front().ranking.rank() * MAX_RANK_RATIO <= results.back().ranking.rank() && results.back().ranking.rank() >= MIN_RANK) {
                    break;
                }
                results.pop_back();
            }
        }

        _entityQueryCounter++;
    }

    std::vector<Geocoder::Name> Geocoder::matchTokenNames(TokenType type, const std::vector<std::vector<Token>>& tokensList, const std::vector<std::string>& tokenStrings) const {
        std::string key(1, static_cast<char>(type));
        for (const std::string& tokenString : tokenStrings) {
            key += tokenString + std::string(1, 0);
        }

        std::vector<Name> names;
        if (_nameMatchCache.read(key, names)) {
            return names;
        }

        _matchTokensCounter++;

        // Now select names based on tokens
        std::map<std::uint64_t, Name> namesUnion;
        for (const std::vector<Token>& tokens : tokensList) {
            std::map<std::uint64_t, Name> names;
            for (std::size_t i = 0; i < tokens.size(); i++) {
                std::string sql = "SELECT DISTINCT n.id, n.name, n.lang, n.xmask, n.ymask FROM names n, nametokens nt WHERE nt.name_id=n.id AND (n.lang IS NULL OR n.lang='" + escapeSQLValue(_language) + "') AND nt.token_id=" + boost::lexical_cast<std::string>(tokens[i].id) + " AND n.class=" + boost::lexical_cast<std::string>(static_cast<int>(type)) + " ORDER BY id ASC";

                std::vector<Name> tokenNames;
                if (!_nameQueryCache.read(sql, tokenNames)) {
                    sqlite3pp::query sqlQuery(_db, sql.c_str());

                    for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                        Name name;
                        name.id = qit->get<std::uint64_t>(0);
                        name.name = qit->get<const char*>(1);
                        name.lang = qit->get<const char*>(2) ? qit->get<const char*>(2) : "";
                        name.xmask = qit->get<std::uint64_t>(3);
                        name.ymask = qit->get<std::uint64_t>(4);
                        name.idf = tokens[i].idf;
                        tokenNames.push_back(name);
                    }

                    _nameQueryCounter++;
                    _nameQueryCache.put(sql, tokenNames);
                }

                for (const Name& name : tokenNames) {
                    names[name.id] = name;
                }
            }

            // Calculate union
            if (namesUnion.empty()) {
                namesUnion = std::move(names);
            }
            else {
                auto it1 = namesUnion.begin();
                auto it2 = names.begin();
                while (it1 != namesUnion.end() && it2 != names.end()) {
                    if (it1->first < it2->first) {
                        it1 = namesUnion.erase(it1);
                    }
                    else if (it1->first > it2->first) {
                        it2++;
                    }
                    else {
                        it1->second.idf += it2->second.idf;
                        it1++; it2++;
                    }
                }
                namesUnion.erase(it1, namesUnion.end());
            }

            if (namesUnion.empty()) {
                _nameMatchCache.put(key, std::vector<Name>());
                return std::vector<Name>(); // fast out
            }
        }

        // Remove names with very low IDF
        for (auto it = namesUnion.begin(); it != namesUnion.end(); it++) {
            if (it->second.idf >= MIN_IDF_THRESHOLD) {
                names.push_back(it->second);
            }
        }

        _nameMatchCache.put(key, names);
        return names;
    }

    std::vector<std::string> Geocoder::buildQueryFilters(const Query& query) const {
        static const std::vector<std::pair<std::string, std::vector<Name> Query::*>> namedFields = {
            { "name_id",          &Query::names },
            { "postcode_id",      &Query::postcodes },
            { "street_id",        &Query::streets },
            { "neighbourhood_id", &Query::neighbourhoods },
            { "locality_id",      &Query::localities },
            { "county_id",        &Query::counties },
            { "region_id",        &Query::regions },
            { "country_id",       &Query::countries }
        };

        bool nonNullField = false;
        std::vector<std::string> sqlFilters;
        for (const std::pair<std::string, std::vector<Name> Query::*>& namedField : namedFields) {
            std::string columnName = namedField.first;
            if (!(query.*namedField.second).empty()) {
                std::string values;
                for (const Name& name : query.*namedField.second) {
                    values += (values.empty() ? "" : ",") + boost::lexical_cast<std::string>(name.id);
                }
                sqlFilters.push_back(columnName + " IN (" + values + ")");
                nonNullField = true;
            }
            else if (!nonNullField && columnName != "name_id" && columnName != "postcode_id") {
                sqlFilters.push_back(columnName + " IS NULL");
            }
        }
        sqlFilters.push_back(!query.houseNumber.empty() ? "housenumbers IS NOT NULL" : "housenumbers IS NULL");
        if (!_enabledFilters.empty()) {
            sqlFilters.push_back(Address::buildTypeFilter(_enabledFilters));
        }
        return sqlFilters;
    }

    bool Geocoder::optimizeQuery(Query& query) const {
        static const std::vector<std::vector<Name> Query::*> namedFields = {
            &Query::names,
            &Query::postcodes,
            &Query::streets,
            &Query::neighbourhoods,
            &Query::localities,
            &Query::counties,
            &Query::regions,
            &Query::countries
        };

        bool progress = true;
        while (progress) {
            progress = false;

            for (std::size_t i = 0; i < namedFields.size(); i++) {
                std::uint64_t xmask = std::numeric_limits<std::uint64_t>::max();
                std::uint64_t ymask = std::numeric_limits<std::uint64_t>::max();
                for (std::size_t j = 0; j < namedFields.size(); j++) {
                    if (i != j && !(query.*namedFields[j]).empty()) {
                        std::uint64_t xmask2 = 0;
                        std::uint64_t ymask2 = 0;
                        for (const Name& name : query.*namedFields[j]) {
                            xmask2 |= name.xmask;
                            ymask2 |= name.ymask;
                        }
                        xmask &= xmask2;
                        ymask &= ymask2;
                    }
                }

                if (!(query.*namedFields[i]).empty()) {
                    for (auto it = (query.*namedFields[i]).begin(); it != (query.*namedFields[i]).end(); ) {
                        if (!(it->xmask & xmask) || !(it->ymask & ymask)) {
                            it = (query.*namedFields[i]).erase(it);
                            progress = true;
                        }
                        else {
                            it++;
                        }
                    }
                    if ((query.*namedFields[i]).empty()) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool Geocoder::bindQueryNameField(Query& query, TokenType type, std::vector<Name> Query::* field, const TokenListType::Span& span) const {
        std::vector<std::string> tokenStrings = query.tokenList.tokens(span);
        if (!tokenStrings.empty() && !tokenStrings.back().empty() && tokenStrings.back().size() <= MIN_AUTOCOMPLETE_SIZE && tokenStrings.back().back() == '%') {
            tokenStrings.back().pop_back(); // too short name, skip autocomplete
        }

        query.tokenList.assignType(span, type);
        if (span.count > 0) {
            std::vector<std::vector<Token>> tokensList = query.tokenList.tags(span);
            std::vector<Name> names = matchTokenNames(type, tokensList, tokenStrings);
            for (const Name& name : names) {
                (query.*field).push_back(name);
            }
            return !names.empty();
        }
        return true;
    }

    bool Geocoder::bindQueryStringField(Query& query, TokenType type, std::string Query::* field, const TokenListType::Span& span) const {
        std::vector<std::string> tokenStrings = query.tokenList.tokens(span);
        if (!tokenStrings.empty() && !tokenStrings.back().empty() && tokenStrings.back().back() == '%') {
            tokenStrings.back().pop_back(); // we do not support autocomplete for 'exact' matching
        }

        query.tokenList.assignType(span, type);
        if (span.count > 0) {
            if (type == TokenType::HOUSENUMBER) {
                TokenListType::Span streetSpan = query.tokenList.span(TokenType::STREET);
                if (!(streetSpan.index == span.index + span.count || streetSpan.index + streetSpan.count == span.index)) {
                    return false;
                }
            }
        }
        query.*field = boost::algorithm::join(tokenStrings, " ");
        return true;
    }

    float Geocoder::calculateNameRank(const std::string& name, const std::string& queryName) const {
        float rank = 1.0f;
        std::string key = name + std::string(1, 0) + queryName;
        if (!_nameRankCache.read(key, rank)) {
            StringMatcher<unistring> matcher(std::bind(&Geocoder::getTokenRank, this, std::placeholders::_1));
            matcher.setMaxDist(MAX_STRINGMATCH_DIST);
            matcher.setTranslationTable(_translationTable, TRANSLATION_EXTRA_PENALTY);
            if (_autocomplete) {
                matcher.setWildcardChar('%', AUTOCOMPLETE_EXTRA_CHAR_PENALTY);
            }
            rank = matcher.calculateRating(toLower(toUniString(queryName)), toLower(toUniString(name)));
            _nameRankCounter++;
            _nameRankCache.put(key, rank);
        }
        return rank;
    }

    float Geocoder::calculateMatchRank(TokenType type, const std::vector<Name>& matchNames, std::uint64_t dbId, const TokenListType& tokenList) const {
        if (std::find_if(matchNames.begin(), matchNames.end(), [dbId](const Name& name) { return name.id == dbId; }) != matchNames.end()) {
            std::string sql = "SELECT DISTINCT n.id, n.name, n.lang FROM names n WHERE n.id=" + boost::lexical_cast<std::string>(dbId) + " AND (n.lang IS NULL OR n.lang='" + escapeSQLValue(_language) + "') AND n.class=" + boost::lexical_cast<std::string>(static_cast<int>(type));

            std::vector<Name> names;
            if (!_nameQueryCache.read(sql, names)) {
                sqlite3pp::query sqlQuery(_db, sql.c_str());

                for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                    Name name;
                    name.id = qit->get<std::uint64_t>(0);
                    name.name = qit->get<const char*>(1);
                    name.lang = qit->get<const char*>(2) ? qit->get<const char*>(2) : "";
                    names.push_back(name);
                }

                _nameQueryCounter++;
                _nameQueryCache.put(sql, names);
            }

            float maxRank = 0.0f;
            for (const Name& name : names) {
                std::string queryName = tokenList.name(type);
                float rank = calculateNameRank(name.name, queryName);
                maxRank = std::max(maxRank, rank);
            }
            return maxRank;
        }
        return dbId ? EXTRA_FIELD_PENALTY : 1.0f;
    }

    float Geocoder::calculatePopulationRank(TokenType type, std::uint64_t id) const {
        static const std::unordered_map<int, std::uint64_t> extraPopulation = {
            { static_cast<int>(TokenType::STREET),             10 },
            { static_cast<int>(TokenType::NEIGHBOURHOOD),     100 },
            { static_cast<int>(TokenType::LOCALITY),         1000 },
            { static_cast<int>(TokenType::COUNTY),          10000 },
            { static_cast<int>(TokenType::REGION),         100000 },
            { static_cast<int>(TokenType::COUNTRY),       1000000 }
        };

        if (!id) {
            return 1.0f;
        }

        auto it = extraPopulation.find(static_cast<int>(type));
        if (it == extraPopulation.end()) {
            return 1.0f;
        }
        std::uint64_t population = 0;
        std::string sql = "SELECT population FROM nameattrs WHERE name_id=" + boost::lexical_cast<std::string>(id);
        if (!_populationCache.read(sql, population)) {
            sqlite3pp::query sqlQuery(_db, sql.c_str());
            for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                population = qit->get<std::uint64_t>(0);
            }
            _populationQueryCounter++;
            _populationCache.put(sql, population);
        }
        return 1.0f - 1.0f / (population + it->second);
    }

    float Geocoder::getTokenRank(const unistring& unitoken) const {
        float idf = 1.0f;
        std::string token = toUtf8String(unitoken);
        if (!_tokenIDFCache.read(token, idf)) {
            sqlite3pp::query sqlQuery(_db, "SELECT idf FROM tokens WHERE token=:token COLLATE NOCASE");
            sqlQuery.bind(":token", token.c_str());
            for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                idf = static_cast<float>(qit->get<double>(0));
            }
            _tokenIDFQueryCounter++;
            _tokenIDFCache.put(token, idf);
        }
        return idf;
    }

    cglib::vec2<double> Geocoder::getOrigin() const {
        sqlite3pp::query query(_db, "SELECT value FROM metadata WHERE name='origin'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::string value = qit->get<const char*>(0);

            std::vector<std::string> origin;
            boost::split(origin, value, boost::is_any_of(","), boost::token_compress_off);
            return cglib::vec2<double>(boost::lexical_cast<double>(origin.at(0)), boost::lexical_cast<double>(origin.at(1)));
        }
        return cglib::vec2<double>(0, 0);
    }

    std::unordered_map<unichar_t, unistring> Geocoder::getTranslationTable() const {
        sqlite3pp::query query(_db, "SELECT value FROM metadata WHERE name='translation_table'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::string value = qit->get<const char*>(0);

            std::vector<std::string> translationVector;
            boost::split(translationVector, value, boost::is_any_of(","), boost::token_compress_off);
            std::unordered_map<unichar_t, unistring> translationTable;
            for (const std::string& translation : translationVector) {
                unistring uniTranslation = toUniString(translation);
                if (uniTranslation.size() >= 2 && uniTranslation[1] == ':') {
                    translationTable[uniTranslation[0]] = uniTranslation.substr(2);
                }
            }
            return translationTable;
        }
        return std::unordered_map<unichar_t, unistring>();
    }
} }
