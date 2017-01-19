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
#include <regex>
#include <mutex>

#include <boost/optional.hpp>

#include <stdext/lru_cache.h>

namespace sqlite3pp {
	class database;
}

namespace carto { namespace geocoding {
	class Geocoder final {
	public:
		explicit Geocoder(sqlite3pp::database& db) : _addressCache(ADDRESS_CACHE_SIZE), _populationCache(POPULATION_CACHE_SIZE), _nameRankCache(NAME_RANK_CACHE_SIZE), _tokenIDFCache(TOKEN_IDF_CACHE_SIZE), _emptyEntityQueryCache(EMPTY_ENTITY_QUERY_CACHE_SIZE), _nameQueryCache(NAME_QUERY_CACHE_SIZE), _db(db) { _origin = findOrigin(); _houseNumberRegex = findHouseNumberRegex(); }

		bool getAutocomplete() const;
		void setAutocomplete(bool autocomplete);

		std::string getLanguage() const;
		void setLanguage(const std::string& language);

		boost::optional<cglib::vec2<double>> getLocation() const;
		void setLocation(const boost::optional<cglib::vec2<double>>& location);

		float getLocationRadius() const;
		void setLocationRadius(float radius);

		std::vector<Address> findAddresses(const std::string& queryString) const;

	private:
		struct Ranking {
			float matchRank = 1.0f;
			float populationRank = 1.0f;
			float locationRank = 1.0f;
			float rank() const { return matchRank * populationRank * locationRank; }
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
			long long nameId = 0;
			std::string houseNumber;
			Ranking ranking;
			int pass = 0;
			bool forceExact = true;
		};
		
		struct Result {
			Query query;
			long long encodedRowId = 0;
			std::size_t unmatchedTokens = 0;
			Ranking ranking;
		};

		void resolvePostcode(const Query& query, std::vector<Result>& results) const;
		void resolveCountry(const Query& query, std::vector<Result>& results) const;
		void resolveRegion(const Query& query, std::vector<Result>& results) const;
		void resolveCounty(const Query& query, std::vector<Result>& results) const;
		void resolveLocality(const Query& query, std::vector<Result>& results) const;
		void resolveNeighbourhood(const Query& query, std::vector<Result>& results) const;
		void resolveStreet(const Query& query, std::vector<Result>& results) const;
		void resolveHouseNumber(const Query& query, std::vector<Result>& results) const;
		void resolveNames(const Query& query, std::vector<Result>& results) const;
		void resolveAddress(const Query& query, std::vector<Result>& results) const;

		std::vector<Query> bindQueryResults(const Query& query, const std::string& type, long long Query::* field) const;
		std::vector<Query> bindQueryNames(const Query& query, const std::string& type, std::string Query::* field) const;

		bool testQuery(const Query& query) const;
		bool testQueryRank(const Query& query, std::vector<Result>& results) const;

		std::vector<std::string> buildQueryFilters(const Query& query, bool nullFilters) const;

		float calculateNameRank(const std::string& name, const std::string& token) const;
		float calculateLangRank(const std::string& lang) const;

		float getTokenRank(const unistring& unitoken) const;
		float getPopulationRank(const std::string& type, long long id) const;

		cglib::vec2<double> findOrigin() const;
		boost::optional<std::regex> findHouseNumberRegex() const;

		static constexpr float MIN_RANK = 0.1f;
		static constexpr float MAX_RANK_RATIO = 0.5f;
		static constexpr float EXTRA_FIELD_PENALTY = 0.95f;
		static constexpr float POI_POPULATION_PENALTY = 0.95f;
		static constexpr int MAX_ENTITY_QUERIES = 100;
		static constexpr std::size_t MAX_RESULTS = 100;
		static constexpr std::size_t MIN_AUTOCOMPLETE_SIZE = 3;
		static constexpr std::size_t ADDRESS_CACHE_SIZE = 1024;
		static constexpr std::size_t POPULATION_CACHE_SIZE = 1024;
		static constexpr std::size_t NAME_RANK_CACHE_SIZE = 1024;
		static constexpr std::size_t TOKEN_IDF_CACHE_SIZE = 512;
		static constexpr std::size_t NAME_QUERY_CACHE_SIZE = 512;
		static constexpr std::size_t EMPTY_ENTITY_QUERY_CACHE_SIZE = 8192;

		bool _autocomplete = false;
		std::string _language;
		boost::optional<cglib::vec2<double>> _location;
		float _locationRadius = 100000; // default is 100km

		mutable cache::lru_cache<long long, Address> _addressCache;
		mutable cache::lru_cache<std::string, long long> _populationCache;
		mutable cache::lru_cache<std::string, float> _nameRankCache;
		mutable cache::lru_cache<std::string, float> _tokenIDFCache;
		mutable cache::lru_cache<std::string, bool> _emptyEntityQueryCache;
		mutable cache::lru_cache<std::string, std::unordered_map<long long, Ranking>> _nameQueryCache;
		mutable long long _previousEntityQueryCounter = 0;;
		mutable long long _entityQueryCounter = 0;
		mutable long long _nameQueryCounter = 0;
		mutable long long _tokenQueryCounter = 0;
		mutable long long _populationQueryCounter = 0;
		mutable long long _nameRankCounter = 0;

		cglib::vec2<double> _origin;
		boost::optional<std::regex> _houseNumberRegex;
		sqlite3pp::database& _db;
		mutable std::recursive_mutex _mutex;
	};
} }

#endif
