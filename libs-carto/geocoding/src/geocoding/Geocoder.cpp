#include "Geocoder.h"
#include "FeatureReader.h"
#include "ProjUtils.h"

#include <functional>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <utf8.h>

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
        if (!safeQueryString.empty() && TokenList<std::string, TokenType>::isSeparator(safeQueryString.back())) {
            autocomplete = false;
        }
        if (autocomplete) {
            // Tricky, do not use autocomplete if the query ends with space. Otherwise append % sign that has special meaning
            safeQueryString += (boost::trim_right_copy(queryString) != queryString ? " " : "%");
        }

        Query query;
        query.options = options;
        query.tokenList = TokenList<std::string, TokenType>::build(safeQueryString);
        
        // Resolve the query into results
        std::vector<Result> results;
        _addressLookupCounter = 0;
        resolvePostcode(query, results);

        // If the best result contains unmatched tokens, do second pass with 'inexact' matching
        if (!results.empty()) {
            query = results.front().query;
        }
        if (query.tokenList.size() == 1) {
            query.forceExact = false;
            resolvePostcode(query, results);
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

    void Geocoder::resolvePostcode(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, false)) {
            return;
        }
        
        if (!query.postcodeId) {
            for (const Query& subQuery : bindQueryResults(query, TokenType::POSTCODE, &Query::postcodeId)) {
                resolveCountry(subQuery, results);
            }
        }
        
        resolveCountry(query, results);
    }

    void Geocoder::resolveCountry(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, false)) {
            return;
        }
        
        if (!query.countryId) {
            for (const Query& subQuery : bindQueryResults(query, TokenType::COUNTRY, &Query::countryId)) {
                resolveRegion(subQuery, results);
            }
        }
        
        resolveRegion(query, results);
    }

    void Geocoder::resolveRegion(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, false)) {
            return;
        }
        
        if (!query.regionId) {
            for (const Query& subQuery : bindQueryResults(query, TokenType::REGION, &Query::regionId)) {
                resolveCounty(subQuery, results);
            }
        }
        
        resolveCounty(query, results);
    }

    void Geocoder::resolveCounty(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, false)) {
            return;
        }
        
        if (!query.countyId) {
            for (const Query& subQuery : bindQueryResults(query, TokenType::COUNTY, &Query::countyId)) {
                resolveLocality(subQuery, results);
            }
        }
        
        resolveLocality(query, results);
    }

    void Geocoder::resolveLocality(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, false)) {
            return;
        }
        
        if (!query.localityId) {
            for (const Query& subQuery : bindQueryResults(query, TokenType::LOCALITY, &Query::localityId)) {
                resolveNeighbourhood(subQuery, results);
            }
        }
        
        resolveNeighbourhood(query, results);
    }

    void Geocoder::resolveNeighbourhood(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, true)) {
            return;
        }
        
        if (!query.neighbourhoodId) {
            for (const Query& subQuery : bindQueryResults(query, TokenType::NEIGHBOURHOOD, &Query::neighbourhoodId)) {
                resolveStreet(subQuery, results);
            }
        }
        
        resolveStreet(query, results);
    }

    void Geocoder::resolveStreet(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, true)) {
            return;
        }
        
        if (!query.streetId) {
            for (const Query& subQuery : bindQueryResults(query, TokenType::STREET, &Query::streetId)) {
                resolveHouseNumber(subQuery, results);
            }
        }
        
        resolveHouseNumber(query, results);
    }

    void Geocoder::resolveHouseNumber(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, true)) {
            return;
        }
        
        if (query.streetId && query.houseNumber.empty()) {
            for (const Query& subQuery : bindQueryNames(query, TokenType::HOUSENUMBER, &Query::houseNumber)) {
                // Accept the subquery only if house number preceeds street name or follows it
                TokenList<std::string, TokenType>::Span streetSpan = subQuery.tokenList.span(TokenType::STREET);
                TokenList<std::string, TokenType>::Span houseNumberSpan = subQuery.tokenList.span(TokenType::HOUSENUMBER);
                if (!(streetSpan.index == houseNumberSpan.index + houseNumberSpan.count || streetSpan.index + streetSpan.count == houseNumberSpan.index)) {
                    continue;
                }
                
                // Try to peform regex matching, this should greatly reduce the number of entities lookups
                if (_houseNumberRegex) {
                    if (!std::regex_match(subQuery.houseNumber, *_houseNumberRegex)) {
                        continue;
                    }
                }
                
                resolveNames(subQuery, results);
            }
        }
        
        resolveNames(query, results);
    }

    void Geocoder::resolveNames(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, true)) {
            return;
        }
        
        if (!query.nameId) {
            for (const Query& subQuery : bindQueryResults(query, TokenType::NAME, &Query::nameId)) {
                resolveAddress(subQuery, results);
            }
        }
        
        resolveAddress(query, results);
    }

    void Geocoder::resolveAddress(const Query& query, std::vector<Result>& results) const {
        if (!testQuery(query, false)) {
            return;
        }
        
        // If we already have match that resolves all tokens, skip the current query
        if (!results.empty()) {
            if (results.front().unmatchedTokens < query.tokenList.size()) {
                return;
            }
        }
        
        std::vector<std::string> sqlFilters = buildQueryFilters(query, true);
        if (sqlFilters.empty()) {
            return;
        }

        _addressLookupCounter++;
        
        std::string sql = "SELECT id, housenumbers, features, country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id, name_id FROM entities WHERE ";
        for (std::size_t i = 0; i < sqlFilters.size(); i++) {
            sql += (i > 0 ? " AND (" : "(") + sqlFilters[i] + ")";
        }

        bool state = false;
        if (_emptyEntityQueryCache.read(sql, state)) {
            return;
        }

        auto mercatorConverter = [this](const cglib::vec2<double>& pos) {
            return wgs84ToWebMercator(_origin + pos);
        };

        sqlite3pp::query sqlQuery(_db, sql.c_str());
        for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
            unsigned int entityId = qit->get<unsigned int>(0);
            unsigned int elementIndex = 0;

            // Match house number
            if (!query.houseNumber.empty()) {
                if (auto houseNumbers = qit->get<const char*>(1)) {
                    std::vector<std::string> houseNumberVector;
                    boost::split(houseNumberVector, houseNumbers, boost::is_any_of("|"), boost::token_compress_off);
                    auto it = std::find_if(houseNumberVector.begin(), houseNumberVector.end(), [&query](const std::string& houseNumber) {
                        return toLower(toUniString(houseNumber)) == toLower(toUniString(query.houseNumber));
                    });
                    if (it == houseNumberVector.end()) {
                        continue; // only possible if COLLATE/toLower work differently
                    }
                    elementIndex = static_cast<unsigned int>(it - houseNumberVector.begin()) + 1;
                }
                else {
                    continue;
                }
            }

            // Do field match ranking and population ranking
            float matchRank = 1.0f;
            matchRank *= calculateMatchRank(TokenType::COUNTRY,       query.countryId,       qit->get<std::uint64_t>(3), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::REGION,        query.regionId,        qit->get<std::uint64_t>(4), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::COUNTY,        query.countyId,        qit->get<std::uint64_t>(5), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::LOCALITY,      query.localityId,      qit->get<std::uint64_t>(6), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::NEIGHBOURHOOD, query.neighbourhoodId, qit->get<std::uint64_t>(7), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::STREET,        query.streetId,        qit->get<std::uint64_t>(8), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::POSTCODE,      query.postcodeId,      qit->get<std::uint64_t>(9), query.tokenList);
            matchRank *= calculateMatchRank(TokenType::NAME,          query.nameId,          qit->get<std::uint64_t>(10), query.tokenList);

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
                if (auto encodedFeatures = qit->get<const void*>(2)) {
                    EncodingStream stream(encodedFeatures, qit->column_bytes(2));
                    FeatureReader reader(stream, mercatorConverter);
                    float minDist = query.options.locationRadius;
                    for (unsigned int currentIndex = 1; !stream.eof(); currentIndex++) {
                        for (const Feature& feature : reader.readFeatureCollection()) {
                            if (std::shared_ptr<Geometry> geometry = feature.getGeometry()) {
                                if (!elementIndex || elementIndex == currentIndex) {
                                    cglib::vec2<double> mercatorMeters = webMercatorMeters(*query.options.location);
                                    cglib::vec2<double> mercatorLocation = wgs84ToWebMercator(*query.options.location);
                                    cglib::vec2<double> point = geometry->calculateNearestPoint(mercatorLocation);
                                    cglib::vec2<double> diff = point - mercatorLocation;
                                    float dist = static_cast<float>(cglib::length(cglib::vec2<double>(diff(0) * mercatorMeters(0), diff(1) * mercatorMeters(1))));
                                    minDist = std::min(minDist, dist);
                                }
                            }
                        }
                    }

                    float c = -std::log(MIN_LOCATION_RANK) / query.options.locationRadius;
                    locationRank *= std::exp(-minDist * c);
                }
            }

            // Build the result
            Result result;
            result.query = query;
            result.encodedId = (static_cast<std::uint64_t>(elementIndex) << 32) | entityId;
            result.unmatchedTokens = query.tokenList.size();
            result.ranking.matchRank = matchRank;
            result.ranking.populationRank = populationRank;
            result.ranking.locationRank = locationRank;

            // Check if the same result is already stored
            auto resultIt = std::find_if(results.begin(), results.end(), [&result](const Result& result2) {
                return result.encodedId == result2.encodedId;
            });
            if (resultIt != results.end()) {
                if (resultIt->unmatchedTokens <= result.unmatchedTokens && resultIt->ranking.rank() >= result.ranking.rank()) {
                    continue; // if we have stored the row with better ranking, ignore current
                }
                results.erase(resultIt); // erase the old match, as the new match is better
            }

            // Find position for the result
            resultIt = std::upper_bound(results.begin(), results.end(), result, [](const Result& result1, const Result& result2) {
                return std::make_tuple(result1.unmatchedTokens, -result1.ranking.rank()) < std::make_tuple(result2.unmatchedTokens, -result2.ranking.rank());
            });
            results.insert(resultIt, result);
            
            // Drop results that have too low rankings
            while (!results.empty()) {
                if (results.front().ranking.rank() * MAX_RANK_RATIO <= results.back().ranking.rank() && results.front().unmatchedTokens == results.back().unmatchedTokens && results.back().ranking.rank() >= MIN_RANK) {
                    break;
                }
                results.pop_back();
            }
        }

        _entityQueryCounter++;
        if (sqlQuery.begin() == sqlQuery.end()) {
            _emptyEntityQueryCache.put(sql, true);
        }
    }

    bool Geocoder::testQuery(const Query& query, bool check) const {
        if (_addressLookupCounter >= MAX_ADDRESS_LOOKUPS) {
            return false;
        }

        if (!check || (!query.streetId && !query.localityId && !query.nameId)) {
            return true;
        }

        std::vector<std::string> sqlFilters = buildQueryFilters(query, false);
        if (sqlFilters.empty()) {
            return true;
        }

        std::string sql = "SELECT 1 FROM entities WHERE ";
        for (std::size_t i = 0; i < sqlFilters.size(); i++) {
            sql += (i > 0 ? " AND (" : "(") + sqlFilters[i] + ")";
        }
        sql += " LIMIT 1";

        bool state = false;
        if (_emptyEntityQueryCache.read(sql, state)) {
            return state;
        }

        sqlite3pp::query sqlQuery(_db, sql.c_str());
        for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
            state = qit->get<std::uint64_t>(0) > 0;
        }

        _emptyEntityQueryCache.put(sql, state);
        return state;
    }

    std::vector<std::string> Geocoder::buildQueryFilters(const Query& query, bool nullFilters) const {
        static const std::vector<std::pair<std::string, std::uint64_t Query::*>> namedFields = {
            { "name_id",          &Query::nameId },
            { "postcode_id",      &Query::postcodeId },
            { "street_id",        &Query::streetId },
            { "neighbourhood_id", &Query::neighbourhoodId },
            { "locality_id",      &Query::localityId },
            { "county_id",        &Query::countyId },
            { "region_id",        &Query::regionId },
            { "country_id",       &Query::countryId }
        };

        bool nonNullField = false;
        std::vector<std::string> sqlFilters;
        for (const std::pair<std::string, std::uint64_t Query::*>& namedField : namedFields) {
            std::string columnName = namedField.first;
            if (query.*namedField.second) {
                sqlFilters.push_back(columnName + "=" + boost::lexical_cast<std::string>(query.*namedField.second));
                nonNullField = true;
            }
            else if (nullFilters && !nonNullField && columnName != "name_id" && columnName != "postcode_id") {
                sqlFilters.push_back(columnName + " IS NULL");
            }
        }
        if (!query.houseNumber.empty()) {
            std::string houseNumber = boost::replace_all_copy(query.houseNumber, "|", " ");
            sqlFilters.push_back("('|' || housenumbers || '|') LIKE '%|" + escapeSQLValue(houseNumber) + "|%' COLLATE NOCASE");
        }
        else if (nullFilters) {
            sqlFilters.push_back("housenumbers IS NULL");
        }
        if (!_enabledFilters.empty()) {
            sqlFilters.push_back(Address::buildTypeFilter(_enabledFilters));
        }
        return sqlFilters;
    }

    std::vector<Geocoder::Query> Geocoder::bindQueryResults(const Query& query, TokenType type, std::uint64_t Query::* field) const {
        std::vector<Query> subQueries;
        for (const TokenList<std::string, TokenType>::Span& span : query.tokenList.enumerate()) {
            std::vector<std::string> tokenStrings = query.tokenList.tokens(span);
            if (!tokenStrings.empty() && !tokenStrings.back().empty() && tokenStrings.back().size() <= MIN_AUTOCOMPLETE_SIZE && tokenStrings.back().back() == '%') {
                tokenStrings.back().pop_back(); // too short name, skip autocomplete
            }

            std::vector<Name> names = matchTokenNames(tokenStrings, type, query.forceExact);
            std::set<std::uint64_t> ids;
            for (const Name& name : names) {
                if (ids.find(name.id) == ids.end()) {
                    Query subQuery(query);
                    subQuery.*field = name.id;
                    subQuery.tokenList.mark(span, type);
                    subQueries.push_back(std::move(subQuery));
                    ids.insert(name.id);
                }
            }
        }
        return subQueries;
    }

    std::vector<Geocoder::Query> Geocoder::bindQueryNames(const Query& query, TokenType type, std::string Query::* field) const {
        std::vector<Query> subQueries;
        for (const TokenList<std::string, TokenType>::Span& span : query.tokenList.enumerate()) {
            std::vector<std::string> tokenStrings = query.tokenList.tokens(span);
            if (!tokenStrings.empty() && !tokenStrings.back().empty() && tokenStrings.back().back() == '%') {
                tokenStrings.back().pop_back(); // we do not support autocomplete for 'exact' matching
            }

            Query subQuery(query);
            subQuery.*field = boost::algorithm::join(tokenStrings, " ");
            subQuery.tokenList.mark(span, type);
            subQueries.push_back(std::move(subQuery));
        }
        return subQueries;
    }

    std::vector<Geocoder::Name> Geocoder::matchTokenNames(const std::vector<std::string>& tokenStrings, TokenType type, bool exactMatch) const {
        // Resolve tokens
        std::vector<std::vector<Token>> tokensList;
        for (std::size_t index = 0; index < tokenStrings.size(); index++) {
            // Do token translation, actual tokens are normalized relative to real names using translation table
            unistring tokenString;
            for (unichar_t c : toUniString(tokenStrings[index])) {
                auto it = _translationTable.find(c);
                if (it != _translationTable.end()) {
                    tokenString += it->second;
                }
                else {
                    tokenString.append(1, c);
                }
            }

            // Get token ids first
            std::string sql = "SELECT id, idf FROM tokens WHERE ";
            if (!exactMatch && tokenString.size() >= 2) {
                sql += "token LIKE '" + escapeSQLValue(toUtf8String(tokenString.substr(0, 2))) + "%' COLLATE NOCASE";
            }
            else if (!tokenString.empty() && tokenString.back() == '%') {
                sql += "token LIKE '" + escapeSQLValue(toUtf8String(tokenString)) + "' COLLATE NOCASE";
            }
            else {
                sql += "token='" + escapeSQLValue(toUtf8String(tokenString)) + "' COLLATE NOCASE";
            }
            sql += " ORDER BY idf DESC";

            std::vector<Token> tokens;
            if (!_tokenQueryCache.read(sql, tokens)) {
                sqlite3pp::query sqlQuery(_db, sql.c_str());

                for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                    Token token;
                    token.id = qit->get<std::uint64_t>(0);
                    token.idf = static_cast<float>(qit->get<double>(1));
                    tokens.push_back(token);
                }

                _tokenQueryCounter++;
                _tokenQueryCache.put(sql, tokens);
            }

            if (tokens.empty()) {
                return std::vector<Name>(); // fast out
            }
            tokensList.push_back(tokens);
        }

        // Sort tokens by minimum IDF
        std::sort(tokensList.begin(), tokensList.end(), [](const std::vector<Token>& tokens1, const std::vector<Token>& tokens2) {
            return tokens1.back().idf > tokens2.back().idf;
        });

        // Now select names based on tokens
        std::vector<Name> namesUnion;
        for (const std::vector<Token>& tokens : tokensList) {
            std::string sql = "SELECT DISTINCT n.id, n.name, n.lang FROM names n, nametokens nt WHERE nt.name_id=n.id AND (n.lang IS NULL OR n.lang='" + escapeSQLValue(_language) + "') AND nt.token_id IN (";
            float idf = 0.0f;
            for (std::size_t i = 0; i < tokens.size(); i++) {
                sql += (i > 0 ? "," : "") + boost::lexical_cast<std::string>(tokens[i].id);
                idf += tokens[i].idf;
            }
            sql += ") AND n.class=" + boost::lexical_cast<std::string>(static_cast<int>(type)) + " ORDER BY id ASC";

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

            // Calculate union
            if (namesUnion.empty()) {
                namesUnion = names;
            }
            else {
                std::size_t i1 = 0, i2 = 0;
                while (i1 < namesUnion.size() && i2 < names.size()) {
                    if (namesUnion[i1].id < names[i2].id) {
                        namesUnion.erase(namesUnion.begin() + i1);
                    }
                    else if (namesUnion[i1].id > names[i2].id) {
                        i2++;
                    }
                    else {
                        i1++; i2++;
                    }
                }
                namesUnion.resize(i1);
            }

            if (namesUnion.empty()) {
                return std::vector<Name>(); // fast out
            }
        }
        return namesUnion;
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

    float Geocoder::calculateMatchRank(TokenType type, std::uint64_t matchId, std::uint64_t dbId, const TokenList<std::string, TokenType>& tokenList) const {
        if (matchId == dbId) {
            if (!dbId) {
                return 1.0f;
            }
            
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
        return EXTRA_FIELD_PENALTY;
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

    boost::optional<std::regex> Geocoder::getHouseNumberRegex() const {
        sqlite3pp::query query(_db, "SELECT value FROM metadata WHERE name='housenumber_regex'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::string value = qit->get<const char*>(0);

            return std::regex(("^(" + value + ")$").c_str(), std::regex_constants::icase);
        }
        return boost::optional<std::regex>();
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
