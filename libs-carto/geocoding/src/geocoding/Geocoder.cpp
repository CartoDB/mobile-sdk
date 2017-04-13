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
    void Geocoder::prepare(sqlite3pp::database& db) {
        sqlite3pp::query query1(db, "SELECT value FROM metadata WHERE name='__entityfilter_data'");
        for (auto qit1 = query1.begin(); qit1 != query1.end(); qit1++) {
            return;
        }

        sqlite3pp::query query2(db, "SELECT COUNT(*) FROM entities");
        std::size_t rowCount = 0;
        for (auto qit2 = query2.begin(); qit2 != query2.end(); qit2++) {
            rowCount = static_cast<std::size_t>(qit2->get<std::uint64_t>(0));
        }

        std::size_t filterBitCount = static_cast<std::size_t>(std::ceil((rowCount * 16 * std::log(ENTITY_BLOOM_FILTER_FP_PROB)) / std::log(1.0 / (std::pow(2.0, std::log(2.0)))))) / (8 * sizeof(EntityFilter::bitset_type::block_type)) * (8 * sizeof(EntityFilter::bitset_type::block_type));
        EntityFilter entityFilter(filterBitCount);
        sqlite3pp::query query(db, "SELECT IFNULL(country_id, 0), IFNULL(region_id, 0), IFNULL(county_id, 0), IFNULL(locality_id, 0), IFNULL(neighbourhood_id, 0), IFNULL(street_id, 0), IFNULL(name_id, 0) FROM entities");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            EntityKey key;
            for (int countryIdx = 0; countryIdx <= (qit->get<std::uint64_t>(0) ? 1 : 0); countryIdx++) {
                key.countryId = qit->get<std::uint64_t>(0) * countryIdx;
                for (int regionIdx = 0; regionIdx <= (qit->get<std::uint64_t>(1) ? 1 : 0); regionIdx++) {
                    key.regionId = qit->get<std::uint64_t>(1) * regionIdx;
                    for (int countyIdx = 0; countyIdx <= (qit->get<std::uint64_t>(2) ? 1 : 0); countyIdx++) {
                        key.countyId = qit->get<std::uint64_t>(2) * countyIdx;
                        for (int localityIdx = 0; localityIdx <= (qit->get<std::uint64_t>(3) ? 1 : 0); localityIdx++) {
                            key.localityId = qit->get<std::uint64_t>(3) * localityIdx;
                            for (int neighbourhoodIdx = 0; neighbourhoodIdx <= (qit->get<std::uint64_t>(4) ? 1 : 0); neighbourhoodIdx++) {
                                key.neighbourhoodId = qit->get<std::uint64_t>(4) * neighbourhoodIdx;
                                for (int streetIdx = 0; streetIdx <= (qit->get<std::uint64_t>(5) ? 1 : 0); streetIdx++) {
                                    key.streetId = qit->get<std::uint64_t>(5) * streetIdx;
                                    for (int nameIdx = 0; nameIdx <= (qit->get<std::uint64_t>(6) ? 1 : 0); nameIdx++) {
                                        key.nameId = qit->get<std::uint64_t>(6) * nameIdx;
                                        entityFilter.insert(key);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        std::vector<EntityFilter::bitset_type::block_type> blocks(entityFilter.data().num_blocks());
        boost::to_block_range(entityFilter.data(), blocks.begin());
        sqlite3pp::command command(db, "INSERT INTO metadata (name, value) VALUES('__entityfilter_data', :value)");
        command.bind(":value", blocks.data(), static_cast<int>(blocks.size() * sizeof(EntityFilter::bitset_type::block_type)));
        command.execute();
    }
    
    bool Geocoder::import(const std::shared_ptr<sqlite3pp::database>& db) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        auto database = std::make_shared<Database>();
        database->id = "db" + boost::lexical_cast<std::string>(_databases.size());
        database->db = db;
        database->origin = getOrigin(*db);
        database->translationTable = getTranslationTable(*db);
        database->entityFilter = EntityFilter(0);

        sqlite3pp::query query(*db, "SELECT value FROM metadata WHERE name='__entityfilter_data'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::size_t size = qit->column_bytes(0);
            const unsigned char* data = static_cast<const unsigned char*>(qit->get<const void*>(0));
            database->entityFilter = EntityFilter(size * 8);
            boost::from_block_range(reinterpret_cast<const EntityFilter::bitset_type::block_type*>(data), reinterpret_cast<const EntityFilter::bitset_type::block_type*>(data + size), database->entityFilter.data());
        }
        
        _databases.push_back(std::move(database));
        return true;
    }
    
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
        _entityCache.clear();
        _nameCache.clear();
        _nameRankCache.clear();
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

        std::string queryStringLC = toUtf8String(toLower(toUniString(queryString)));
        std::string safeQueryString = boost::replace_all_copy(boost::replace_all_copy(queryStringLC, "%", ""), "_", "");
        boost::trim(safeQueryString);

        // Prepare autocomplete query string by appending % sign
        bool autocomplete = _autocomplete && safeQueryString.size() >= MIN_AUTOCOMPLETE_SIZE;
        if (!safeQueryString.empty() && TokenList::isSeparator(safeQueryString.back())) {
            autocomplete = false;
        }
        if (autocomplete) {
            // Tricky, do not use autocomplete if the query ends with space. Otherwise append % sign that has special meaning
            safeQueryString += (boost::trim_right_copy(queryString) != queryString ? " " : "%");
        }

        // Do matching in 2 phases (exact/inexact), if required
        std::vector<Result> results;
        for (int pass = 0; pass < 2; pass++) {
            for (const std::shared_ptr<Database>& database : _databases) {
                Query query;
                query.database = database;
                query.tokenList = TokenList::build(safeQueryString);
                matchTokens(query, pass, query.tokenList);
                matchQuery(query, options, results);
            }
            if (!results.empty()) {
                break;
            }
        }

        // Reorder databases, keep databases with best matches first in the list for subsequent queries
        for (auto it = results.rbegin(); it != results.rend(); it++) {
            auto dbit = std::find(_databases.begin(), _databases.end(), it->database);
            if (dbit != _databases.end()) {
                std::rotate(_databases.begin(), dbit, dbit + 1);
            }
        }

        // Create address data from the results by merging consecutive results, if possible
        std::vector<std::pair<Address, float>> addresses;
        for (const Result& result : results) {
            if (addresses.size() >= MAX_RESULTS) {
                break;
            }

            Address address;
            std::string addrKey = result.database->id + std::string(1, 0) + boost::lexical_cast<std::string>(result.encodedId);
            if (!_addressCache.read(addrKey, address)) {
                address.loadFromDB(*result.database->db, result.encodedId, _language, [&result](const cglib::vec2<double>& pos) {
                    return result.database->origin + pos;
                });

                _addressQueryCounter++;
                _addressCache.put(addrKey, address);
            }

            if (!addresses.empty() && result.totalRank() == addresses.back().second) {
                if (addresses.back().first.merge(address)) {
                    continue;
                }
            }
            addresses.emplace_back(address, result.totalRank());
        }
        return addresses;
    }

    void Geocoder::matchTokens(Query& query, int pass, TokenList& tokenList) const {
        for (int i = 0; i < tokenList.size(); i++) {
            std::string tokenValue = tokenList.tokens(TokenList::Span(i, 1)).front();

            // Do token translation, actual tokens are normalized relative to real names using translation table
            unistring translatedToken = getTranslatedToken(toUniString(tokenValue), query.database->translationTable);

            // Build token info list for the token
            if (!translatedToken.empty()) {
                std::string sql = "SELECT id, token, typemask, idf FROM tokens WHERE ";
                if (pass > 0 && translatedToken.size() >= 2) {
                    sql += "token LIKE '" + escapeSQLValue(toUtf8String(translatedToken.substr(0, 2))) + "%' ORDER BY ABS(LENGTH(token) - " + boost::lexical_cast<std::string>(translatedToken.size()) + ") ASC, idf ASC LIMIT 10";
                }
                else if (!translatedToken.empty() && translatedToken.back() == '%') {
                    sql += "token LIKE '" + escapeSQLValue(toUtf8String(translatedToken)) + "' ORDER BY LENGTH(token) ASC, idf ASC LIMIT 10";
                }
                else {
                    sql += "token='" + escapeSQLValue(toUtf8String(translatedToken)) + "'";
                }
                sqlite3pp::query sqlQuery(*query.database->db, sql.c_str());

                std::vector<Token> tokens;
                std::uint32_t validTypeMask = 0;
                float minIDF = std::numeric_limits<float>::infinity();
                for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                    Token token;
                    token.id = qit->get<std::uint64_t>(0);
                    token.token = qit->get<const char*>(1);
                    token.typeMask = qit->get<std::uint32_t>(2);
                    token.idf = static_cast<float>(qit->get<double>(3));
                    
                    std::map<unistring, float> tokenIDFMap;
                    tokenIDFMap[toUniString(token.token)] = token.idf;
                    if (calculateNameRank(query, token.token, toUtf8String(translatedToken), tokenIDFMap) >= MIN_MATCH_THRESHOLD) {
                        tokens.push_back(std::move(token));
                        minIDF = std::min(minIDF, token.idf);
                        validTypeMask |= token.typeMask;
                    }
                }
                tokenList.setTag(i, tokens);
                tokenList.setIDF(i, minIDF);
                tokenList.setValidTypeMask(i, validTypeMask);
            }
        }
    }

    void Geocoder::matchQuery(Query& query, const Options& options, std::vector<Result>& results) const {
        if (!results.empty()) {
            if (results.front().unmatchedTokens < query.tokenList.unmatchedInvalidTokens()) {
                return;
            }
        }
        
        // Enumerate token list
        std::uint32_t validTypeMask = (1 << static_cast<int>(FieldType::COUNTRY)) | (1 << static_cast<int>(FieldType::REGION)) | (1 << static_cast<int>(FieldType::COUNTY)) | (1 << static_cast<int>(FieldType::LOCALITY)) | (1 << static_cast<int>(FieldType::NEIGHBOURHOOD)) | (1 << static_cast<int>(FieldType::STREET)) | (1 << static_cast<int>(FieldType::NAME));
        std::uint32_t strictTypeMask = (1 << static_cast<int>(FieldType::STREET)) | (1 << static_cast<int>(FieldType::NAME));
        if (query.streets) {
            validTypeMask |= 1 << static_cast<int>(FieldType::HOUSENUMBER);
        }
        if (query.houseNumber) {
            validTypeMask |= 1 << static_cast<int>(FieldType::POSTCODE);
        }
        
        FieldType type = FieldType::NONE;
        std::vector<TokenList::Span> spans;
        query.tokenList.enumerateSpans(validTypeMask, strictTypeMask, type, spans);
        if (type == FieldType::NONE) {
            resolveQuery(query, options, results);
            return;
        }
        
        // Resolve/bind enumerated tokens to subqueries and recurse
        std::vector<TokenList::Span> validSpans;
        for (const TokenList::Span& span : spans) {
            if (span.count > 0) {
                bool skip = false;
                for (const TokenList::Span& validSpan : validSpans) {
                    if (span.index >= validSpan.index && span.index + span.count <= validSpan.index + validSpan.count) {
                        skip = true;
                        break;
                    }
                }
                if (skip) {
                    continue;
                }
            }

            Query subQuery = query;
            subQuery.tokenList.assignType(span, type);
            if (!subQuery.tokenList.valid()) {
                continue;
            }

            bool valid = true;
            if (span.count > 0) {
                switch (type) {
                case FieldType::HOUSENUMBER:
                    valid = bindQueryStringField(subQuery, type, &Query::houseNumber, span);
                    break;
                default:
                    valid = bindQueryNameField(subQuery, type, Query::getField(type), span);
                    break;
                }

                if (valid) {
                    valid = filterQuery(subQuery, results);
                    if (valid) {
                        validSpans.push_back(span);
                    }
                }
            }

            if (valid) {
                matchQuery(subQuery, options, results);
            }
        }
    }

    void Geocoder::matchNames(const Query& query, FieldType type, const std::vector<std::vector<Token>>& tokensList, const std::string& matchName, std::shared_ptr<std::vector<Geocoder::NameRank>>& nameRanks) const {
        std::string nameKey = query.database->id + std::string(1, 0) + static_cast<char>(type) + std::string(1, 0) + matchName;
        for (const std::vector<Token>& tokens : tokensList) {
            nameKey += std::string(1, 0);
            for (const Token& token : tokens) {
                nameKey += boost::lexical_cast<std::string>(token.id) + ";";
            }
        }
        if (!_nameRankCache.read(nameKey, nameRanks)) {
            // Select names based on tokens
            std::vector<std::vector<std::shared_ptr<Name>>> namesList;
            for (const std::vector<Token>& tokens : tokensList) {
                std::string sql = "SELECT DISTINCT n.id, n.name, n.lang, n.type FROM names n, nametokens nt WHERE nt.name_id=n.id AND (n.lang IS NULL OR n.lang='" + escapeSQLValue(_language) + "') AND nt.token_id IN (";
                for (std::size_t i = 0; i < tokens.size(); i++) {
                    sql += (i > 0 ? "," : "") + boost::lexical_cast<std::string>(tokens[i].id);
                }
                sql += ") ORDER BY id ASC";

                std::vector<std::shared_ptr<Name>> names;
                std::string queryKey = query.database->id + std::string(1, 0) + sql;
                if (!_nameCache.read(queryKey, names)) {
                    sqlite3pp::query sqlQuery(*query.database->db, sql.c_str());

                    for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                        auto name = std::make_shared<Name>();
                        name->id = qit->get<std::uint64_t>(0);
                        name->name = qit->get<const char*>(1);
                        name->lang = qit->get<const char*>(2) ? qit->get<const char*>(2) : "";
                        name->type = static_cast<FieldType>(qit->get<int>(3));

                        std::string sql2 = "SELECT t.token, t.idf FROM tokens t, nametokens nt WHERE t.id=nt.token_id AND nt.name_id=" + boost::lexical_cast<std::string>(name->id);
                        sqlite3pp::query sqlQuery2(*query.database->db, sql2.c_str());
                        for (auto qit2 = sqlQuery2.begin(); qit2 != sqlQuery2.end(); qit2++) {
                            std::string nameToken = qit2->get<const char*>(0);
                            float idf = static_cast<float>(qit2->get<double>(1));
                            name->tokenIDFs.emplace_back(nameToken, idf);
                        }

                        names.push_back(std::move(name));
                    }

                    _nameQueryCounter++;
                    _nameCache.put(queryKey, names);
                }

                namesList.push_back(std::move(names));
            }

            // Match names, use binary search for fast merging
            nameRanks = std::make_shared<std::vector<NameRank>>();
            if (!namesList.empty()) {
                std::sort(namesList.begin(), namesList.end(), [](const std::vector<std::shared_ptr<Name>>& names1, const std::vector<std::shared_ptr<Name>>& names2) {
                    return names1.size() < names2.size();
                });

                for (std::size_t i = 0; i < namesList[0].size(); i++) {
                    std::map<unistring, float> tokenIDFMap;
                    bool match = namesList[0][i]->type == type;
                    float idf = 0;
                    for (const std::pair<std::string, float>& tokenIDF : namesList[0][i]->tokenIDFs) {
                        tokenIDFMap[toUniString(tokenIDF.first)] = tokenIDF.second;
                        idf += tokenIDF.second;
                    }
                    for (std::size_t j = 1; j < namesList.size(); j++) {
                        auto it = std::lower_bound(namesList[j].begin(), namesList[j].end(), namesList[0][i], [](const std::shared_ptr<Name>& name1, const std::shared_ptr<Name>& name2) {
                            return name1->id < name2->id;
                        });
                        if (it == namesList[j].end() || (*it)->id != namesList[0][i]->id) {
                            match = false;
                            break;
                        }
                        for (const std::pair<std::string, float>& tokenIDF : (*it)->tokenIDFs) {
                            tokenIDFMap[toUniString(tokenIDF.first)] = tokenIDF.second;
                            idf += tokenIDF.second;
                        }
                    }
                    if (match && idf >= MIN_IDF_THRESHOLD) {
                        float rank = calculateNameRank(query, namesList[0][i]->name, matchName, tokenIDFMap);
                        if (rank >= MIN_MATCH_THRESHOLD) {
                            nameRanks->push_back(NameRank { namesList[0][i], rank });
                        }
                    }
                }
            }

            // Sort the results by decreasing ranks
            std::sort(nameRanks->begin(), nameRanks->end(), [](const NameRank& nameRank1, const NameRank& nameRank2) {
                return nameRank1.rank > nameRank2.rank;
            });

            _nameRankCounter++;
            _nameRankCache.put(nameKey, nameRanks);
        }
    }

    void Geocoder::resolveQuery(const Query& query, const Options& options, std::vector<Result>& results) const {
        if (query.null()) {
            return;
        }
        
        const Database& database = *query.database;
        std::vector<std::string> sqlFilters = buildQueryFilters(query);
        if (sqlFilters.empty()) {
            return;
        }
        std::string sql = "SELECT DISTINCT id, housenumbers, features, country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id, name_id, rank FROM entities WHERE ";
        for (std::size_t i = 0; i < sqlFilters.size(); i++) {
            sql += (i > 0 ? " AND (" : "(") + sqlFilters[i] + ")";
        }

        std::string entityKey = database.id + std::string(1, 0) + sql;
        std::vector<EntityRow> entityRows;
        if (!_entityCache.read(entityKey, entityRows)) {
            sqlite3pp::query sqlQuery(*database.db, sql.c_str());
            for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
                EntityRow entityRow;
                entityRow.id = qit->get<unsigned int>(0);
                if (qit->get<const char*>(1)) {
                    entityRow.houseNumbers = qit->get<const char*>(1);
                }
                if (qit->get<const void*>(2)) {
                    entityRow.features = std::string(static_cast<const char*>(qit->get<const void*>(2)), qit->column_bytes(2));
                }
                entityRow.countryId = qit->get<std::uint64_t>(3);
                entityRow.regionId = qit->get<std::uint64_t>(4);
                entityRow.countyId = qit->get<std::uint64_t>(5);
                entityRow.localityId = qit->get<std::uint64_t>(6);
                entityRow.neighbourhoodId = qit->get<std::uint64_t>(7);
                entityRow.streetId = qit->get<std::uint64_t>(8);
                entityRow.postcodeId = qit->get<std::uint64_t>(9);
                entityRow.nameId = qit->get<std::uint64_t>(10);
                entityRow.rank = static_cast<float>(qit->get<int>(11)) / 32767.0f;
                entityRows.push_back(std::move(entityRow));
            }

            _entityQueryCounter++;
            _entityCache.put(entityKey, entityRows);
            _missingEntityQueryCounter += (entityRows.empty() ? 1 : 0);
        }

        auto mercatorConverter = [&database](const cglib::vec2<double>& pos) {
            return wgs84ToWebMercator(database.origin + pos);
        };

        for (const EntityRow& entityRow : entityRows) {
            // Match house number
            unsigned int elementIndex = 0;
            if (query.houseNumber) {
                AddressInterpolator interpolator(entityRow.houseNumbers);
                elementIndex = interpolator.findAddress(*query.houseNumber) + 1; // if not found, interpolator returns -1
                if (!elementIndex) {
                    continue;
                }
            }

            // Build the result
            Result result;
            result.database = query.database;
            result.encodedId = (static_cast<std::uint64_t>(elementIndex) << 32) | entityRow.id;
            result.unmatchedTokens = query.unmatchedTokens();

            // Do field match ranking
            updateMatchRank(database, FieldType::COUNTRY, query, entityRow.countryId, result);
            updateMatchRank(database, FieldType::REGION, query, entityRow.regionId, result);
            updateMatchRank(database, FieldType::COUNTY, query, entityRow.countyId, result);
            updateMatchRank(database, FieldType::LOCALITY, query, entityRow.localityId, result);
            updateMatchRank(database, FieldType::NEIGHBOURHOOD, query, entityRow.neighbourhoodId, result);
            updateMatchRank(database, FieldType::STREET, query, entityRow.streetId, result);
            updateMatchRank(database, FieldType::POSTCODE, query, entityRow.postcodeId, result);
            updateMatchRank(database, FieldType::NAME, query, entityRow.nameId, result);

            // Set penalty for unmatched fields
            result.matchRank *= std::pow(UNMATCHED_FIELD_PENALTY, query.tokenList.unmatchedTokens());

            // Set entity ranking
            result.entityRank *= entityRow.rank;

            // Do location based ranking
            if (options.location) {
                EncodingStream stream(entityRow.features.data(), entityRow.features.size());
                FeatureReader reader(stream, mercatorConverter);

                std::vector<Feature> features;
                if (elementIndex) {
                    AddressInterpolator interpolator(entityRow.houseNumbers);
                    features = interpolator.enumerateAddresses(reader).at(elementIndex - 1).second;
                }
                else {
                    features = reader.readFeatureCollection();
                }

                updateLocationRank(database, options, features, result);
            }

            // Early out test
            if (result.totalRank() < MIN_RANK) {
                continue;
            }

            // Check if the same result is already stored
            auto resultIt = std::find_if(results.begin(), results.end(), [&result](const Result& result2) {
                return result.encodedId == result2.encodedId;
            });
            if (resultIt != results.end()) {
                if (resultIt->totalRank() >= result.totalRank()) {
                    continue; // if we have stored the row with better ranking, ignore current
                }
                results.erase(resultIt); // erase the old match, as the new match is better
            }

            // Find position for the result
            resultIt = std::upper_bound(results.begin(), results.end(), result, [](const Result& result1, const Result& result2) {
                return result1.totalRank() > result2.totalRank();
            });
            if (!(resultIt == results.end() && results.size() == MAX_RESULTS)) {
                results.insert(resultIt, result);

                // Drop results that have too low rankings
                while (!results.empty()) {
                    if (results.front().totalRank() * MAX_RANK_RATIO <= results.back().totalRank() && results.back().totalRank() >= MIN_RANK) {
                        break;
                    }
                    results.pop_back();
                }
            }
        }
    }

    float Geocoder::calculateNameRank(const Query& query, const std::string& name, const std::string& queryName, const std::map<unistring, float>& tokenIDFMap) const {
        float rank = 1.0f;
        std::string nameKey = query.database->id + std::string(1, 0) + name + std::string(1, 0) + queryName;
        if (!_nameMatchCache.read(nameKey, rank)) {
            auto getTokenRank = [&tokenIDFMap, &query](const unistring& token) {
                unistring translatedToken = getTranslatedToken(token, query.database->translationTable);
                auto it = tokenIDFMap.find(translatedToken);
                if (it != tokenIDFMap.end()) {
                    return it->second;
                }
                return 1.0f;
            };

            StringMatcher<unistring> matcher(getTokenRank);
            matcher.setMaxDist(MAX_STRINGMATCH_DIST);
            matcher.setTranslationTable(query.database->translationTable, TRANSLATION_EXTRA_PENALTY);
            if (_autocomplete) {
                matcher.setWildcardChar('%', AUTOCOMPLETE_EXTRA_CHAR_PENALTY);
            }
            rank = matcher.calculateRating(toLower(toUniString(queryName)), toLower(toUniString(name)));

            _nameMatchCounter++;
            _nameMatchCache.put(nameKey, rank);
        }
        return rank;
    }

    std::vector<std::string> Geocoder::buildQueryFilters(const Query& query) const {
        static const std::vector<std::pair<std::string, std::shared_ptr<std::vector<NameRank>> Query::*>> namedFields = {
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
        for (const std::pair<std::string, std::shared_ptr<std::vector<NameRank>> Query::*>& namedField : namedFields) {
            std::string columnName = namedField.first;
            if (query.*namedField.second) {
                std::string values;
                for (const NameRank& nameRank : *(query.*namedField.second)) {
                    values += (values.empty() ? "" : ",") + boost::lexical_cast<std::string>(nameRank.name->id);
                }
                sqlFilters.push_back(columnName + " IN (" + values + ")");
                nonNullField = true;
            }
            else if (!nonNullField && columnName != "name_id" && columnName != "postcode_id") {
                sqlFilters.push_back(columnName + " IS NULL");
            }
        }
        sqlFilters.push_back(query.houseNumber ? "housenumbers IS NOT NULL" : "housenumbers IS NULL");
        if (!_enabledFilters.empty()) {
            sqlFilters.push_back(Address::buildTypeFilter(_enabledFilters));
        }
        return sqlFilters;
    }

    void Geocoder::updateMatchRank(const Database& database, FieldType type, const Query& query, std::uint64_t dbId, Result& result) const {
        const std::shared_ptr<std::vector<NameRank>>& matchNames = query.*Query::getField(type);
        if (matchNames) {
            auto it = std::find_if(matchNames->begin(), matchNames->end(), [dbId](const NameRank& nameRank) {
                return nameRank.name->id == dbId;
            });
            if (it != matchNames->end()) {
                result.matchRank *= it->rank;
            }
        }
        else if (dbId) {
            result.matchRank *= EXTRA_FIELD_PENALTY;
        }
    }

    void Geocoder::updateLocationRank(const Database& database, const Options& options, const std::vector<Feature>& features, Result& result) const {
        float minDist = options.locationRadius;
        for (const Feature& feature : features) {
            if (std::shared_ptr<Geometry> geometry = feature.getGeometry()) {
                cglib::vec2<double> mercatorMeters = webMercatorMeters(*options.location);
                cglib::vec2<double> mercatorLocation = wgs84ToWebMercator(*options.location);
                cglib::vec2<double> point = geometry->calculateNearestPoint(mercatorLocation);
                cglib::vec2<double> diff = point - mercatorLocation;
                float dist = static_cast<float>(cglib::length(cglib::vec2<double>(diff(0) * mercatorMeters(0), diff(1) * mercatorMeters(1))));
                minDist = std::min(minDist, dist);
            }
        }

        float c = -std::log(MIN_LOCATION_RANK) / options.locationRadius;
        result.locationRank *= std::exp(-minDist * c);
    }

    bool Geocoder::filterQuery(Query& query, const std::vector<Result>& results) const {
        float bestRank = (results.empty() ? 0.0f : results.front().totalRank());
        float worstRank = (results.empty() ? 0.0f : results.back().totalRank());
        for (int i = static_cast<int>(FieldType::COUNTRY); i < static_cast<int>(FieldType::HOUSENUMBER); i++) {
            FieldType type = static_cast<FieldType>(i);
            std::shared_ptr<std::vector<NameRank>> Query::* field = query.getField(type);
            if (query.*field) {
                float rank = !(query.*field)->empty() ? query.bestRank(type, 0) : 0.0f;
                if ((rank < MIN_RANK) || (rank < bestRank * MAX_RANK_RATIO) || (rank < worstRank && results.size() >= MAX_RESULTS)) {
                    return false;
                }

                bool copy = false;
                while (!(query.*field)->empty()) {
                    float rank = query.bestRank(type, (query.*field)->size() - 1);
                    if (!(rank < MIN_RANK) && !(rank < bestRank * MAX_RANK_RATIO) && !(rank < worstRank && results.size() >= MAX_RESULTS)) {
                        break;
                    }
                    if (!copy) {
                        query.*field = std::make_shared<std::vector<NameRank>>(*(query.*field));
                        copy = true;
                    }
                    (query.*field)->pop_back();
                }
                if ((query.*field)->empty()) {
                    return false;
                }
            }
        }

        auto iterateKeys = [](const std::shared_ptr<std::vector<NameRank>>& names) -> const std::vector<NameRank>& {
            static const std::vector<NameRank> nullNames = { NameRank { std::make_shared<Name>(), 1.0f } };
            if (!names) {
                return nullNames;
            }
            return *names;
        };

        auto filterKeys = [](std::shared_ptr<std::vector<NameRank>>& names, const std::unordered_set<uint64_t>& ids) -> bool {
            if (!names) {
                return true;
            }
            std::shared_ptr<std::vector<NameRank>> filteredNames;
            for (auto it = names->begin(); it != names->end(); it++) {
                if (ids.count((*it).name->id)) {
                    if (filteredNames) {
                        filteredNames->push_back(*it);
                    }
                }
                else {
                    if (!filteredNames) {
                        filteredNames = std::make_shared<std::vector<NameRank>>(names->begin(), it);
                    }
                }
            }
            if (!filteredNames) {
                return true;
            }
            names = std::move(filteredNames);
            return !names->empty();
        };

        std::unordered_set<std::uint64_t> filteredIndices[7];
        EntityKey key;
        for (const NameRank& country : iterateKeys(query.countries)) {
            key.countryId = country.name->id;
            for (const NameRank& region : iterateKeys(query.regions)) {
                key.regionId = region.name->id;
                for (const NameRank& county : iterateKeys(query.counties)) {
                    key.countyId = county.name->id;
                    for (const NameRank& locality : iterateKeys(query.localities)) {
                        key.localityId = locality.name->id;
                        for (const NameRank& neighbourhood : iterateKeys(query.neighbourhoods)) {
                            key.neighbourhoodId = neighbourhood.name->id;
                            for (const NameRank& street : iterateKeys(query.streets)) {
                                key.streetId = street.name->id;
                                for (const NameRank& name : iterateKeys(query.names)) {
                                    key.nameId = name.name->id;
                                    if (query.database->entityFilter.data().empty() || query.database->entityFilter.probably_contains(key)) {
                                        filteredIndices[0].insert(country.name->id);
                                        filteredIndices[1].insert(region.name->id);
                                        filteredIndices[2].insert(county.name->id);
                                        filteredIndices[3].insert(locality.name->id);
                                        filteredIndices[4].insert(neighbourhood.name->id);
                                        filteredIndices[5].insert(street.name->id);
                                        filteredIndices[6].insert(name.name->id);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return filterKeys(query.countries, filteredIndices[0]) && filterKeys(query.regions, filteredIndices[1]) && filterKeys(query.counties, filteredIndices[2]) && filterKeys(query.localities, filteredIndices[3]) && filterKeys(query.neighbourhoods, filteredIndices[4]) && filterKeys(query.streets, filteredIndices[5]) && filterKeys(query.names, filteredIndices[6]);
    }

    bool Geocoder::bindQueryNameField(Query& query, FieldType type, std::shared_ptr<std::vector<NameRank>> Query::* field, const TokenList::Span& span) const {
        std::vector<std::vector<Token>> tokensList = query.tokenList.tags(span);
        std::string matchName = boost::algorithm::join(query.tokenList.tokens(span), " ");
        matchNames(query, type, tokensList, matchName, query.*field);
        return !(query.*field)->empty();
    }

    bool Geocoder::bindQueryStringField(Query& query, FieldType type, std::shared_ptr<std::string> Query::* field, const TokenList::Span& span) const {
        std::vector<std::string> tokenStrings = query.tokenList.tokens(span);
        if (!tokenStrings.empty() && !tokenStrings.back().empty() && tokenStrings.back().back() == '%') {
            tokenStrings.back().pop_back(); // we do not support autocomplete for 'exact' matching
        }

        if (type == FieldType::HOUSENUMBER) {
            if (query.tokenList.prevType(span) != FieldType::STREET && query.tokenList.nextType(span) != FieldType::STREET) {
                return false;
            }
        }
        query.*field = std::make_shared<std::string>(boost::algorithm::join(tokenStrings, " "));
        return true;
    }

    cglib::vec2<double> Geocoder::getOrigin(sqlite3pp::database& db) {
        sqlite3pp::query query(db, "SELECT value FROM metadata WHERE name='origin'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::string value = qit->get<const char*>(0);

            std::vector<std::string> origin;
            boost::split(origin, value, boost::is_any_of(","), boost::token_compress_off);
            return cglib::vec2<double>(boost::lexical_cast<double>(origin.at(0)), boost::lexical_cast<double>(origin.at(1)));
        }
        return cglib::vec2<double>(0, 0);
    }

    std::unordered_map<unichar_t, unistring> Geocoder::getTranslationTable(sqlite3pp::database& db) {
        sqlite3pp::query query(db, "SELECT value FROM metadata WHERE name='translation_table'");
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

    unistring Geocoder::getTranslatedToken(const unistring& token, const std::unordered_map<unichar_t, unistring>& translationTable) {
        unistring translatedToken;
        translatedToken.reserve(token.size());
        for (unichar_t c : token) {
            auto it = translationTable.find(c);
            if (it != translationTable.end()) {
                translatedToken += it->second;
            }
            else {
                translatedToken.append(1, c);
            }
        }
        return translatedToken;
    }
} }
