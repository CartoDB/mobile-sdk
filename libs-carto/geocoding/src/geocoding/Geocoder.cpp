#include "Geocoder.h"
#include "FeatureReader.h"
#include "ProjUtils.h"

#include <functional>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
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
	}

	std::vector<std::pair<Address, float>> Geocoder::findAddresses(const std::string& queryString, const Options& options) const {
		std::lock_guard<std::recursive_mutex> lock(_mutex);

		std::string safeQueryString = boost::replace_all_copy(boost::replace_all_copy(queryString, "%", ""), "_", "");
		boost::trim(safeQueryString);

		bool autocomplete = _autocomplete && safeQueryString.size() >= MIN_AUTOCOMPLETE_SIZE;
		if (!safeQueryString.empty() && TokenList<std::string>::isSeparator(safeQueryString.back())) {
			autocomplete = false;
		}

		Query query;
		query.options = options;
		query.tokenList = TokenList<std::string>::build(autocomplete ? safeQueryString + "%" : safeQueryString);
		
		// Resolve the query into results
		std::vector<Result> results;
		_previousEntityQueryCounter = _entityQueryCounter;
		resolvePostcode(query, results);

		// If the best result contains unmatched tokens, do second pass with 'inexact' matching
		if (!results.empty()) {
			query = results.front().query;
		}
		if (query.tokenList.size() == 1) {
			results.clear();
			query.ranking = Ranking();
			query.pass++;
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
		if (!testQueryRank(query, results)) {
			return;
		}
		if (!query.postcodeId) {
			for (const Query& subQuery : bindQueryResults(query, "postcode", &Query::postcodeId)) {
				resolveCountry(subQuery, results);
			}
		}
		resolveCountry(query, results);
	}

	void Geocoder::resolveCountry(const Query& query, std::vector<Result>& results) const {
		if (!testQueryRank(query, results)) {
			return;
		}
		if (!query.countryId) {
			for (const Query& subQuery : bindQueryResults(query, "country", &Query::countryId)) {
				resolveRegion(subQuery, results);
			}
		}
		resolveRegion(query, results);
	}

	void Geocoder::resolveRegion(const Query& query, std::vector<Result>& results) const {
		if (!testQueryRank(query, results)) {
			return;
		}
		if (!query.regionId) {
			for (const Query& subQuery : bindQueryResults(query, "region", &Query::regionId)) {
				resolveCounty(subQuery, results);
			}
		}
		resolveCounty(query, results);
	}

	void Geocoder::resolveCounty(const Query& query, std::vector<Result>& results) const {
		if (!testQueryRank(query, results)) {
			return;
		}
		if (!query.countyId) {
			for (const Query& subQuery : bindQueryResults(query, "county", &Query::countyId)) {
				resolveLocality(subQuery, results);
			}
		}
		resolveLocality(query, results);
	}

	void Geocoder::resolveLocality(const Query& query, std::vector<Result>& results) const {
		if (!testQueryRank(query, results)) {
			return;
		}
		if (!query.localityId) {
			for (const Query& subQuery : bindQueryResults(query, "locality", &Query::localityId)) {
				resolveNeighbourhood(subQuery, results);
			}
		}
		resolveNeighbourhood(query, results);
	}

	void Geocoder::resolveNeighbourhood(const Query& query, std::vector<Result>& results) const {
		if (!testQueryRank(query, results) || !testQuery(query)) {
			return;
		}
		if (!query.neighbourhoodId) {
			for (const Query& subQuery : bindQueryResults(query, "neighbourhood", &Query::neighbourhoodId)) {
				resolveStreet(subQuery, results);
			}
		}
		resolveStreet(query, results);
	}

	void Geocoder::resolveStreet(const Query& query, std::vector<Result>& results) const {
		if (!testQueryRank(query, results) || !testQuery(query)) {
			return;
		}
		if (!query.streetId) {
			for (const Query& subQuery : bindQueryResults(query, "street", &Query::streetId)) {
				resolveHouseNumber(subQuery, results);
			}
		}
		resolveHouseNumber(query, results);
	}

	void Geocoder::resolveHouseNumber(const Query& query, std::vector<Result>& results) const {
		if (!testQueryRank(query, results) || !testQuery(query)) {
			return;
		}
		if (query.streetId && query.houseNumber.empty()) {
			for (const Query& subQuery : bindQueryNames(query, "house", &Query::houseNumber)) {
				// Accept the subquery only if house number preceeds street name or follows it
				TokenList<std::string>::Span streetSpan = subQuery.tokenList.span("street");
				TokenList<std::string>::Span houseNumberSpan = subQuery.tokenList.span("house");
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
		if (!testQueryRank(query, results) || !testQuery(query)) {
			return;
		}
		if (!query.nameId) {
			for (const Query& subQuery : bindQueryResults(query, "name", &Query::nameId)) {
				resolveAddress(subQuery, results);
			}
		}
		resolveAddress(query, results);
	}

	void Geocoder::resolveAddress(const Query& query, std::vector<Result>& results) const {
		// If we already have match that resolves all tokens, skip the current query
		if (!results.empty()) {
			if (results.front().unmatchedTokens < query.tokenList.size()) {
				return;
			}
		}
		if (!testQueryRank(query, results)) {
			return;
		}
		
		std::vector<std::string> sqlFilters = buildQueryFilters(query, true);
		if (sqlFilters.empty()) {
			return;
		}

		std::string sql = "SELECT id, housenumbers, features, country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id, name_id FROM entities WHERE ";
		for (std::size_t i = 0; i < sqlFilters.size(); i++) {
			if (i > 0) {
				sql += " AND ";
			}
			sql += "(" + sqlFilters[i] + ")";
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
			matchRank *= (query.countryId       == qit->get<std::uint64_t>(3) ? 1.0f : EXTRA_FIELD_PENALTY);
			matchRank *= (query.regionId        == qit->get<std::uint64_t>(4) ? 1.0f : EXTRA_FIELD_PENALTY);
			matchRank *= (query.countyId        == qit->get<std::uint64_t>(5) ? 1.0f : EXTRA_FIELD_PENALTY);
			matchRank *= (query.localityId      == qit->get<std::uint64_t>(6) ? 1.0f : EXTRA_FIELD_PENALTY);
			matchRank *= (query.neighbourhoodId == qit->get<std::uint64_t>(7) ? 1.0f : EXTRA_FIELD_PENALTY);
			matchRank *= (query.streetId        == qit->get<std::uint64_t>(8) ? 1.0f : EXTRA_FIELD_PENALTY);
			matchRank *= (query.postcodeId      == qit->get<std::uint64_t>(9) ? 1.0f : EXTRA_FIELD_PENALTY);
			matchRank *= (query.nameId          == qit->get<std::uint64_t>(10) ? 1.0f : EXTRA_FIELD_PENALTY);

			float populationRank = 1.0f;
			populationRank *= getPopulationRank("country",       qit->get<std::uint64_t>(3));
			populationRank *= getPopulationRank("region",        qit->get<std::uint64_t>(4));
			populationRank *= getPopulationRank("county",        qit->get<std::uint64_t>(5));
			populationRank *= getPopulationRank("locality",      qit->get<std::uint64_t>(6));
			populationRank *= getPopulationRank("neighbourhood", qit->get<std::uint64_t>(7));
			populationRank *= getPopulationRank("street",        qit->get<std::uint64_t>(8));
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
			result.ranking = query.ranking;
			result.ranking.matchRank *= matchRank;
			result.ranking.populationRank *= populationRank;
			result.ranking.locationRank *= locationRank;

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

	std::vector<Geocoder::Query> Geocoder::bindQueryResults(const Query& query, const std::string& type, std::uint64_t Query::* field) const {
		std::vector<Query> subQueries;
		for (const TokenList<std::string>::Span& span : query.tokenList.enumerate()) {
			std::string name = query.tokenList.tokens(span);
			if (name.size() > 0 && name.size() <= MIN_AUTOCOMPLETE_SIZE && name.back() == '%') {
				name = name.substr(0, name.size() - 1); // too short name, skip autocomplete
			}

			std::string::iterator nameIt = name.begin();
			if (nameIt != name.end()) {
				utf8::next(nameIt, name.end());
			}
			if (nameIt != name.end()) {
				utf8::next(nameIt, name.end());
			}

			// Get token ids first
			std::string sql = "SELECT t.id FROM tokens t WHERE ";
			if (!query.forceExact && nameIt != name.end()) {
				sql += "t.token LIKE '" + escapeSQLValue(std::string(name.begin(), nameIt)) + "%' COLLATE NOCASE";
			}
			else if (!name.empty() && name.back() == '%') {
				sql += "t.token LIKE '" + escapeSQLValue(name) + "' COLLATE NOCASE";
			}
			else {
				sql += "t.token='" + escapeSQLValue(name) + "' COLLATE NOCASE";
			}
			
			std::vector<std::uint64_t> tokenIds;
			if (!_tokenQueryCache.read(sql, tokenIds)) {
				sqlite3pp::query sqlQuery(_db, sql.c_str());

				for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
					auto id = qit->get<std::uint64_t>(0);
					tokenIds.push_back(id);
				}

				_tokenQueryCounter++;
				_tokenQueryCache.put(sql, tokenIds);
			}
			if (tokenIds.empty()) {
				continue;
			}

			// Now select names based on token ids
			sql = "SELECT DISTINCT tt.id, tn.name, tn.lang FROM " + type + "tokens tt, " + type + "names tn WHERE tt.id=tn.id AND (tn.lang=tt.lang OR (tn.lang IS NULL AND tt.lang IS NULL)) AND tt.token_id IN (";
			for (std::size_t i = 0; i < tokenIds.size(); i++) {
				sql += (i > 0 ? "," : "") + boost::lexical_cast<std::string>(tokenIds[i]);
			}
			sql += ")";

			std::unordered_map<std::uint64_t, Ranking> resultRankings;
			if (!_nameQueryCache.read(sql, resultRankings)) {
				sqlite3pp::query sqlQuery(_db, sql.c_str());

				for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
					auto id = qit->get<std::uint64_t>(0);
					auto realName = qit->get<const char*>(1);
					auto lang = qit->get<const char*>(2);
					float nameRank = calculateNameRank(name, realName);
					if (nameRank == 0) {
						continue;
					}
					float langRank = calculateLangRank(lang ? std::string(lang) : std::string());
					Ranking ranking;
					ranking.matchRank = nameRank * langRank;
					ranking.populationRank = 1.0f;
					if (resultRankings.find(id) == resultRankings.end()) {
						resultRankings[id] = ranking;
					}
					else {
						resultRankings[id].matchRank = std::max(resultRankings[id].matchRank, ranking.matchRank);
						resultRankings[id].populationRank = std::max(resultRankings[id].populationRank, ranking.populationRank);
					}
				}

				_nameQueryCounter++;
				_nameQueryCache.put(sql, resultRankings);
			}

			// Build subqueries
			for (const std::pair<std::uint64_t, Ranking>& resultRanking : resultRankings) {
				Query subQuery(query);
				subQuery.*field = resultRanking.first;
				subQuery.ranking.matchRank *= resultRanking.second.matchRank;
				subQuery.ranking.populationRank *= resultRanking.second.populationRank;
				subQuery.tokenList.mark(span, type);
				subQuery.forceExact = true;
				subQueries.push_back(std::move(subQuery));
			}
		}

		std::sort(subQueries.begin(), subQueries.end(), [](const Query& subQuery1, const Query& subQuery2) {
			return subQuery1.ranking.rank() > subQuery2.ranking.rank();
		});
		return subQueries;
	}

	std::vector<Geocoder::Query> Geocoder::bindQueryNames(const Query& query, const std::string& type, std::string Query::* field) const {
		std::vector<Query> subQueries;
		for (const TokenList<std::string>::Span& span : query.tokenList.enumerate()) {
			std::string name = query.tokenList.tokens(span);
			if (name.size() > 0 && name.back() == '%') {
				name = name.substr(0, name.size() - 1); // we do not support autocomplete for 'exact' matching
			}

			Query subQuery(query);
			subQuery.*field = name;
			subQuery.tokenList.mark(span, type);
			subQueries.push_back(std::move(subQuery));
		}
		return subQueries;
	}

	bool Geocoder::testQuery(const Query& query) const {
		if (!query.streetId && !query.localityId && !query.nameId) {
			return true;
		}

		std::vector<std::string> sqlFilters = buildQueryFilters(query, false);
		if (sqlFilters.empty()) {
			return true;
		}

		std::string sql = "SELECT 1 FROM entities WHERE ";
		for (std::size_t i = 0; i < sqlFilters.size(); i++) {
			if (i > 0) {
				sql += " AND ";
			}
			sql += "(" + sqlFilters[i] + ")";
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
		return sqlFilters;
	}

	bool Geocoder::testQueryRank(const Query& query, std::vector<Result>& results) const {
		if (_entityQueryCounter - _previousEntityQueryCounter > MAX_ENTITY_QUERIES) {
			return false;
		}
		if (query.ranking.rank() < MIN_RANK) {
			return false;
		}
		if (!results.empty()) {
			if (!(results.front().ranking.rank() * MAX_RANK_RATIO <= query.ranking.rank())) {
				return false;
			}
		}
		return true;
	}

	float Geocoder::calculateNameRank(const std::string& name, const std::string& token) const {
		float rank = 1.0f;
		std::string key = name + std::string(1, 0) + token;
		if (!_nameRankCache.read(key, rank)) {
			StringMatcher<unistring> matcher(std::bind(&Geocoder::getTokenRank, this, std::placeholders::_1));
			matcher.setMaxDist(2);
			rank = matcher.calculateRating(toLower(toUniString(name)), toLower(toUniString(token)));
			_nameRankCounter++;
			_nameRankCache.put(key, rank);
		}
		return rank;
	}

	float Geocoder::calculateLangRank(const std::string& lang) const {
		if (lang.empty()) {
			return 1.0f; // use highest ranking for 'default' language
		}
		return _language == lang ? 1.0f : 0.5f;
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

	float Geocoder::getPopulationRank(const std::string& type, std::uint64_t id) const {
		static const std::unordered_map<std::string, std::uint64_t> extraPopulation = {
			{ "street",             10 },
			{ "neighbourhood",     100 },
			{ "locality",         1000 },
			{ "county",          10000 },
			{ "region",         100000 },
			{ "country",       1000000 }
		};

		if (!id) {
			return 1.0f;
		}

		auto it = extraPopulation.find(type);
		if (it == extraPopulation.end()) {
			return 1.0f;
		}
		std::uint64_t population = 0;
		std::string sql = "SELECT population FROM " + type + "attrs WHERE id=" + boost::lexical_cast<std::string>(id);
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

	cglib::vec2<double> Geocoder::findOrigin() const {
		sqlite3pp::query query(_db, "SELECT value FROM metadata WHERE name='origin'");
		for (auto qit = query.begin(); qit != query.end(); qit++) {
			std::string value = qit->get<const char*>(0);

			std::vector<std::string> origin;
			boost::split(origin, value, boost::is_any_of(","), boost::token_compress_off);
			return cglib::vec2<double>(boost::lexical_cast<double>(origin.at(0)), boost::lexical_cast<double>(origin.at(1)));
		}
		return cglib::vec2<double>(0, 0);
	}

	boost::optional<std::regex> Geocoder::findHouseNumberRegex() const {
		sqlite3pp::query query(_db, "SELECT value FROM metadata WHERE name='housenumber_regex'");
		for (auto qit = query.begin(); qit != query.end(); qit++) {
			std::string value = qit->get<const char*>(0);

			return std::regex(("^(" + value + ")$").c_str(), std::regex_constants::icase);
		}
		return boost::optional<std::regex>();
	}
} }
