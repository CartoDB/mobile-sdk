/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_GEOCODER_H_
#define _CARTO_GEOCODING_GEOCODER_H_

#include "Address.h"
#include "TokenList.h"
#include "StringMatcher.h"
#include "StringUtils.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <stdext/lru_cache.h>

namespace sqlite3pp {
	class database;
}

namespace carto { namespace geocoding {
	class Geocoder {
	public:
		explicit Geocoder(sqlite3pp::database& db) : _addressCache(ADDRESS_CACHE_SIZE), _populationCache(POPULATION_CACHE_SIZE), _nameRankCache(NAME_RANK_CACHE_SIZE), _tokenIDFCache(TOKEN_IDF_CACHE_SIZE), _emptyEntityQueryCache(EMPTY_ENTITY_QUERY_CACHE_SIZE), _nameQueryCache(NAME_QUERY_CACHE_SIZE), _db(db) { }

		void setAutocomplete(bool autocomplete) { _autocomplete = autocomplete; }

		std::vector<Address> findAddresses(const std::string& queryString) const;

	private:
		struct Ranking {
			float matchRank = 1.0f;
			float populationRank = 1.0f;
			float rank() const { return matchRank * populationRank; }
		};
		
		struct Query {
			TokenList<std::string> tokenList;
			long long countryId = 0;
			long long regionId = 0;
			long long countyId = 0;
			long long localityId = 0;
			long long neighbourhoodId = 0;
			long long streetId = 0;
			long long postcodeId = 0;
			std::string building;
			std::string name;
			Ranking ranking;
		};
		
		struct Result {
			long long encodedRowId = 0;
			Ranking ranking;
		};

		void resolveCountry(Query query, std::vector<Result>& results) const;
		void resolveRegion(Query query, std::vector<Result>& results) const;
		void resolveCounty(Query query, std::vector<Result>& results) const;
		void resolveLocality(Query query, std::vector<Result>& results) const;
		void resolveNeighbourhood(Query query, std::vector<Result>& results) const;
		void resolveStreet(Query query, std::vector<Result>& results) const;
		void resolvePostcode(const Query& query, std::vector<Result>& results) const;
		void resolveBuilding(const Query& query, std::vector<Result>& results) const;
		void resolveNames(const Query& query, std::vector<Result>& results) const;
		void resolveRemaining(Query query, std::vector<Result>& results) const;
		void resolveAddress(const Query& query, std::vector<Result>& results) const;

		std::vector<Query> bindQueryResults(const Query& query, const std::string& type, long long Query::* field) const;
		std::vector<Query> bindQueryNames(const Query& query, const std::string& type, std::string Query::* field) const;

		float calculateNameRank(const std::string& name, const std::string& token) const;
		float calculateLangRank(const std::string& lang) const;

		float getTokenRank(const unistring& unitoken) const;
		float getPopulationRank(const std::string& type, long long id) const;

		static constexpr float MIN_RANK = 0.1f;
		static constexpr float MAX_RANK_RATIO = 0.5f;
		static constexpr std::size_t MAX_RESULTS = 100;
		static constexpr std::size_t MIN_AUTOCOMPLETE_SIZE = 3;
		static constexpr std::size_t ADDRESS_CACHE_SIZE = 1024;
		static constexpr std::size_t POPULATION_CACHE_SIZE = 1024;
		static constexpr std::size_t NAME_RANK_CACHE_SIZE = 1024;
		static constexpr std::size_t TOKEN_IDF_CACHE_SIZE = 512;
		static constexpr std::size_t NAME_QUERY_CACHE_SIZE = 512;
		static constexpr std::size_t EMPTY_ENTITY_QUERY_CACHE_SIZE = 2048;

		bool _autocomplete = false;
		mutable cache::lru_cache<long long, Address> _addressCache;
		mutable cache::lru_cache<std::string, long long> _populationCache;
		mutable cache::lru_cache<std::string, float> _nameRankCache;
		mutable cache::lru_cache<std::string, float> _tokenIDFCache;
		mutable cache::lru_cache<std::string, bool> _emptyEntityQueryCache;
		mutable cache::lru_cache<std::string, std::unordered_map<long long, Ranking>> _nameQueryCache;
		mutable long long _entityQueryCounter = 0;
		mutable long long _nameQueryCounter = 0;
		mutable long long _tokenQueryCounter = 0;
		mutable long long _populationQueryCounter = 0;
		mutable long long _nameRankCounter = 0;
		sqlite3pp::database& _db;
		mutable std::recursive_mutex _mutex;
	};
} }

#endif
