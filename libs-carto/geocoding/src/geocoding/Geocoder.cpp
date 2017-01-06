#include "Geocoder.h"

#include <functional>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>

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

	std::vector<Address> Geocoder::findAddresses(const std::string& queryString) const {
		std::lock_guard<std::recursive_mutex> lock(_mutex);

		std::string safeQueryString = boost::replace_all_copy(boost::replace_all_copy(queryString, "'", ""), "%", "");

		Query query;
		query.tokenList = TokenList<std::string>::build(safeQueryString);
		if (_autocomplete && !safeQueryString.empty()) {
			char lastChar = safeQueryString.back();
			const std::string& lastToken = query.tokenList.at(query.tokenList.size() - 1).first;
			if (lastToken.size() >= MIN_AUTOCOMPLETE_SIZE && !TokenList<std::string>::isSeparator(lastChar) && !TokenList<std::string>::isSpace(lastChar)) {
				query.tokenList = TokenList<std::string>::build(safeQueryString + "%");
			}
		}
		
		std::vector<Result> results;
		resolveCountry(query, results);

		std::vector<Address> addresses;
		for (const Result& result : results) {
			Address address;
			if (!_addressCache.read(result.encodedRowId, address)) {
				address.loadFromDB(_db, result.encodedRowId);
				_addressCache.put(result.encodedRowId, address);
			}
			addresses.push_back(std::move(address));
		}
		return addresses;
	}

	void Geocoder::resolveCountry(Query query, std::vector<Result>& results) const {
		for (const Query& subQuery : bindQueryResults(query, "country", &Query::countryId)) {
			resolveRegion(subQuery, results);
		}
		query.ranking.populationRank *= getPopulationRank("country", 0);
		resolveRegion(query, results);
	}

	void Geocoder::resolveRegion(Query query, std::vector<Result>& results) const {
		for (const Query& subQuery : bindQueryResults(query, "region", &Query::regionId)) {
			resolveCounty(subQuery, results);
		}
		query.ranking.populationRank *= getPopulationRank("region", 0);
		resolveCounty(query, results);
	}

	void Geocoder::resolveCounty(Query query, std::vector<Result>& results) const {
		for (const Query& subQuery : bindQueryResults(query, "county", &Query::countyId)) {
			resolveLocality(subQuery, results);
		}
		query.ranking.populationRank *= getPopulationRank("county", 0);
		resolveLocality(query, results);
	}

	void Geocoder::resolveLocality(Query query, std::vector<Result>& results) const {
		for (const Query& subQuery : bindQueryResults(query, "locality", &Query::localityId)) {
			resolveNeighbourhood(subQuery, results);
		}
		query.ranking.populationRank *= getPopulationRank("locality", 0);
		resolveNeighbourhood(query, results);
	}

	void Geocoder::resolveNeighbourhood(Query query, std::vector<Result>& results) const {
		for (const Query& subQuery : bindQueryResults(query, "neighbourhood", &Query::neighbourhoodId)) {
			resolveStreet(subQuery, results);
		}
		query.ranking.populationRank *= getPopulationRank("neighbourhood", 0);
		resolveStreet(query, results);
	}

	void Geocoder::resolveStreet(Query query, std::vector<Result>& results) const {
		for (const Query& subQuery : bindQueryResults(query, "street", &Query::streetId)) {
			resolvePostcode(subQuery, results);
		}
		query.ranking.populationRank *= getPopulationRank("street", 0);
		resolvePostcode(query, results);
	}

	void Geocoder::resolvePostcode(const Query& query, std::vector<Result>& results) const {
		if (query.streetId) {
			for (const Query& subQuery : bindQueryResults(query, "postcode", &Query::postcodeId)) {
				resolveBuilding(subQuery, results);
			}
		}
		resolveBuilding(query, results);
	}

	void Geocoder::resolveBuilding(const Query& query, std::vector<Result>& results) const {
		if (query.streetId) {
			for (const Query& subQuery : bindQueryNames(query, "building", &Query::building)) {
				TokenList<std::string>::Span streetSpan = subQuery.tokenList.span("street");
				TokenList<std::string>::Span buildingSpan = subQuery.tokenList.span("building");
				if (streetSpan.index == buildingSpan.index + buildingSpan.count || streetSpan.index + streetSpan.count == buildingSpan.index) {
					resolveNames(subQuery, results);
				}
			}
		}
		resolveNames(query, results);
	}

	void Geocoder::resolveNames(const Query& query, std::vector<Result>& results) const {
		for (const Query& subQuery : bindQueryNames(query, "name", &Query::name)) {
			resolveRemaining(subQuery, results);
		}
		resolveRemaining(query, results);
	}

	void Geocoder::resolveRemaining(Query query, std::vector<Result>& results) const {
		float rank = 1.0f;
		for (std::size_t i = 0; i < query.tokenList.size(); i++) {
			rank *= calculateNameRank(query.tokenList.at(i).first, "") * 0.75f;
		}
		query.ranking.matchRank *= rank;
		resolveAddress(query, results);
	}

	void Geocoder::resolveAddress(const Query& query, std::vector<Result>& results) const {
		if (query.ranking.rank() < MIN_RANK) {
			return;
		}
		if (!results.empty()) {
			if (!(results.front().ranking.rank() * MAX_RANK_RATIO <= query.ranking.rank())) {
				return;
			}
		}
		
		std::vector<std::string> sqlFilters, sqlNullFilters;

		std::string name = query.name;
		if (!name.empty()) {
			if (name.size() >= 3) {
				sqlFilters.push_back("name LIKE '" + escapeSQLValue(name) + "%' COLLATE NOCASE OR name LIKE '% " + escapeSQLValue(name) + "%' COLLATE NOCASE");
			}
			else {
				sqlFilters.push_back("name='" + escapeSQLValue(name) + "'");
			}
		}
		
		std::string building = boost::replace_all_copy(query.building, "|", " ");
		if (!building.empty()) {
			sqlFilters.push_back("('|' || housenums || '|') LIKE '%|" + escapeSQLValue(building) + "|%' COLLATE NOCASE");
		}
		else {
			sqlNullFilters.push_back("housenums IS NULL");
		}

		std::vector<std::pair<std::string, long long Query::*>> namedFields = {
			{ "postcode_id",      &Query::postcodeId },
			{ "street_id",        &Query::streetId },
			{ "neighbourhood_id", &Query::neighbourhoodId },
			{ "locality_id",      &Query::localityId },
			{ "county_id",        &Query::countyId },
			{ "region_id",        &Query::regionId },
			{ "country_id",       &Query::countryId }
		};
		for (const std::pair<std::string, long long Query::*>& namedField : namedFields) {
			if (query.*namedField.second) {
				sqlFilters.push_back(namedField.first + "=" + boost::lexical_cast<std::string>(query.*namedField.second));
			}
			else if (sqlFilters.empty()) {
				sqlNullFilters.push_back(namedField.first + " IS NULL");
			}
		}

		sqlFilters.insert(sqlFilters.end(), sqlNullFilters.begin(), sqlNullFilters.end());
		if (sqlFilters.empty()) {
			sqlFilters.push_back("1=1");
		}

		std::string sql = "SELECT rowid, housenums, name, country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id FROM entities WHERE ";
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

		sqlite3pp::query sqlQuery(_db, sql.c_str());
		for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
			long long rowId = qit->get<long long>(0);

			std::size_t houseId = 0;
			const char* houseNums = qit->get<const char*>(1);
			if (!query.building.empty()) {
				if (houseNums) {
					std::vector<std::string> houseNumsVector;
					boost::split(houseNumsVector, houseNums, boost::is_any_of("|"), boost::token_compress_off);
					auto it = std::find(houseNumsVector.begin(), houseNumsVector.end(), building);
					if (it == houseNumsVector.end()) {
						continue;
					}
					houseId = (it - houseNumsVector.begin()) + 1;
				}
			}

			float matchRank = 1.0f;
			const char* name = qit->get<const char*>(2);
			if (!query.name.empty()) {
				matchRank *= calculateNameRank(query.name, name ? std::string(name) : std::string());
			}
			else {
				matchRank *= (name ? 0.75f : 1.0f); // try to keep entities without names before current entity
			}

			for (int col = 3; col < sqlQuery.column_count(); col++) {
				auto fieldIt = std::find_if(namedFields.begin(), namedFields.end(), [&sqlQuery, col](const std::pair<std::string, long long Query::*>& namedField) {
					return sqlQuery.column_name(col) == namedField.first;
				});
				if (fieldIt != namedFields.end()) {
					matchRank *= (query.*fieldIt->second == qit->get<long long>(col) ? 1.0f : 0.75f);
				}
			}

			Result result;
			result.encodedRowId = (rowId << 32) | houseId;
			result.ranking = query.ranking;
			result.ranking.matchRank *= matchRank;
			
			auto resultIt = std::find_if(results.begin(), results.end(), [&result](const Result& result2) {
				return result.encodedRowId == result2.encodedRowId;
			});
			if (resultIt != results.end()) {
				if (resultIt->ranking.rank() >= result.ranking.rank()) {
					continue;
				}
				results.erase(resultIt);
			}

			resultIt = std::upper_bound(results.begin(), results.end(), result, [](const Result& result1, const Result& result2) {
				return result1.ranking.rank() > result2.ranking.rank();
			});
			results.insert(resultIt, result);
			
			while (!results.empty()) {
				if (results.size() <= MAX_RESULTS && results.front().ranking.rank() * MAX_RANK_RATIO <= results.back().ranking.rank() && results.back().ranking.rank() >= MIN_RANK) {
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

	std::vector<Geocoder::Query> Geocoder::bindQueryResults(const Query& query, const std::string& type, long long Query::* field) const {
		std::vector<Query> subQueries;
		for (const TokenList<std::string>::Span& span : query.tokenList.enumerate()) {
			const std::string& name = query.tokenList.tokens(span);

			std::string sql = "SELECT DISTINCT tt.id, t.token, tt.lang FROM tokens t, " + type + "tokens tt WHERE ";
			if (name.size() >= 3) {
				sql += "t.id = tt.token_id AND t.token LIKE '" + escapeSQLValue(name) + "%' COLLATE NOCASE";
			}
			else {
				sql += "t.id = tt.token_id AND t.token='" + escapeSQLValue(name) + "' COLLATE NOCASE";
			}

			std::unordered_map<long long, Ranking> resultRankings;
			if (!_nameQueryCache.read(sql, resultRankings)) {
				sqlite3pp::query sqlQuery(_db, sql.c_str());

				for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
					long long id = qit->get<long long>(0);
					const char* token = qit->get<const char*>(1);
					const char* lang = qit->get<const char*>(2);
					float nameRank = calculateNameRank(name, token);
					float langRank = calculateLangRank(lang ? std::string(lang) : std::string());
					Ranking ranking;
					ranking.matchRank = nameRank * langRank;
					ranking.populationRank = getPopulationRank(type, id);
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

			for (const std::pair<long long, Ranking>& resultRanking : resultRankings) {
				Query subQuery(query);
				subQuery.*field = resultRanking.first;
				subQuery.ranking.matchRank *= resultRanking.second.matchRank;
				subQuery.ranking.populationRank *= resultRanking.second.populationRank;
				subQuery.tokenList.mark(span, type);
				subQueries.push_back(std::move(subQuery));
			}
		}

		std::sort(subQueries.begin(), subQueries.end(), [](const Query& query1, const Query& query2) {
			return query1.ranking.rank() > query2.ranking.rank();
		});
		return subQueries;
	}

	std::vector<Geocoder::Query> Geocoder::bindQueryNames(const Query& query, const std::string& type, std::string Query::* field) const {
		std::vector<Query> subQueries;
		for (const TokenList<std::string>::Span& span : query.tokenList.enumerate()) {
			Query subQuery(query);
			subQuery.*field = query.tokenList.tokens(span);
			subQuery.tokenList.mark(span, type);
			subQueries.push_back(std::move(subQuery));
		}
		return subQueries;
	}

	float Geocoder::calculateNameRank(const std::string& name, const std::string& token) const {
		float rank = 1.0f;
		std::string key = name + std::string(1, 0) + token;
		if (!_nameRankCache.read(key, rank)) {
			StringMatcher<unistring> matcher(std::bind(&Geocoder::getTokenRank, this, std::placeholders::_1));
			matcher.setMaxDist(2);
			rank = matcher.calculateRating(toLower(toUniString(name)), toLower(toUniString(token))) * 0.5f + 0.5f;
			_nameRankCounter++;
			_nameRankCache.put(key, rank);
		}
		return rank;
	}

	float Geocoder::calculateLangRank(const std::string& lang) const {
		return 1.0f; // TODO: implement language ranking
	}

	float Geocoder::getTokenRank(const unistring& unitoken) const {
		float idf = 1.0f;
		std::string token = toUtf8String(unitoken);
		if (!_tokenIDFCache.read(token, idf)) {
			sqlite3pp::query sqlQuery(_db, "SELECT idf FROM tokens WHERE token=:token");
			sqlQuery.bind(":token", token.c_str());
			for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
				idf = static_cast<float>(qit->get<double>(0));
			}
			_tokenQueryCounter++;
			_tokenIDFCache.put(token, idf);
		}
		return idf;
	}

	float Geocoder::getPopulationRank(const std::string& type, long long id) const {
		static const std::unordered_map<std::string, long long> extraPopulation = {
			{ "street",             10 },
			{ "neighbourhood",     100 },
			{ "locality",         1000 },
			{ "county",          10000 },
			{ "region",         100000 },
			{ "country",       1000000 }
		};

		auto it = extraPopulation.find(type);
		if (it == extraPopulation.end()) {
			return 1.0f;
		}
		long long population = 0;
		std::string sql = "SELECT population FROM " + type + "attrs WHERE id=" + boost::lexical_cast<std::string>(id);
		if (!_populationCache.read(sql, population)) {
			sqlite3pp::query sqlQuery(_db, sql.c_str());
			for (auto qit = sqlQuery.begin(); qit != sqlQuery.end(); qit++) {
				population = qit->get<long long>(0);
			}
			_populationQueryCounter++;
			_populationCache.put(sql, population);
		}
		return 1.0f - 1.0f / (population + it->second);
	}
} }
