/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_GEOCODER_H_
#define _CARTO_GEOCODING_GEOCODER_H_

#include "Address.h"
#include "TaggedTokenList.h"
#include "StringMatcher.h"
#include "StringUtils.h"

#include <string>
#include <regex>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <boost/optional.hpp>
#include <boost/bloom_filter/twohash_dynamic_basic_bloom_filter.hpp>
#include <boost/functional/hash.hpp>

#include <stdext/lru_cache.h>

namespace sqlite3pp {
    class database;
}

namespace carto { namespace geocoding {
    class Geocoder final {
    public:
        struct Options {
            boost::optional<cglib::vec2<double>> location; // default is no location bias
            float locationRadius = 100000; // default is 100km
        };

        Geocoder() : _addressCache(ADDRESS_CACHE_SIZE), _entityCache(ENTITY_CACHE_SIZE), _nameCache(NAME_CACHE_SIZE), _tokenCache(TOKEN_CACHE_SIZE), _nameRankCache(NAME_RANK_CACHE_SIZE), _nameMatchCache(NAME_MATCH_CACHE_SIZE) { }

        static void prepare(sqlite3pp::database& db);

        bool import(const std::shared_ptr<sqlite3pp::database>& db);
        
        bool getAutocomplete() const;
        void setAutocomplete(bool autocomplete);

        std::string getLanguage() const;
        void setLanguage(const std::string& language);

        bool isFilterEnabled(Address::Type type) const;
        void setFilterEnabled(Address::Type type, bool enabled);
        
        std::vector<std::pair<Address, float>> findAddresses(const std::string& queryString, const Options& options) const;

    private:
        enum class FieldType {
            NONE = 0, COUNTRY, REGION, COUNTY, LOCALITY, NEIGHBOURHOOD, STREET, POSTCODE, NAME, HOUSENUMBER
        };

        struct Token {
            std::uint64_t id = 0;
            std::string token;
            std::uint32_t typeMask = 0;
            float idf = 0.0f;
        };
        
        using TokenList = TaggedTokenList<std::string, FieldType, std::vector<Token>>;

        struct Name {
            std::uint64_t id = 0;
            std::string name;
            std::string lang;
            FieldType type = FieldType::NONE;
            std::vector<std::pair<std::string, float>> tokenIDFs;
        };

        struct NameRank {
            std::shared_ptr<Name> name;
            float rank;
        };

        struct EntityRow {
            std::uint64_t id = 0;
            std::uint64_t countryId = 0;
            std::uint64_t regionId = 0;
            std::uint64_t countyId = 0;
            std::uint64_t localityId = 0;
            std::uint64_t neighbourhoodId = 0;
            std::uint64_t streetId = 0;
            std::uint64_t postcodeId = 0;
            std::uint64_t nameId = 0;
            std::string houseNumbers;
            std::string features;
            float rank = 0.0f;
        };

        struct EntityKey {
            std::uint64_t countryId = 0;
            std::uint64_t regionId = 0;
            std::uint64_t countyId = 0;
            std::uint64_t localityId = 0;
            std::uint64_t neighbourhoodId = 0;
            std::uint64_t streetId = 0;
            std::uint64_t postcodeId = 0;
            std::uint64_t nameId = 0;
        };

        using EntityKeyHash1 = boost::bloom_filters::murmurhash3<carto::geocoding::Geocoder::EntityKey, 0, false>;
        using EntityKeyHash2 = boost::bloom_filters::murmurhash3<carto::geocoding::Geocoder::EntityKey, 1, true>;
        using EntityFilter = boost::bloom_filters::twohash_dynamic_basic_bloom_filter<EntityKey, 7, 0, EntityKeyHash1, EntityKeyHash2>;

        struct Database {
            std::string id;
            std::shared_ptr<sqlite3pp::database> db;
            cglib::vec2<double> origin;
            std::unordered_map<unichar_t, unistring> translationTable;
            EntityFilter entityFilter;
        };

        struct Query {
            std::shared_ptr<Database> database;
            TokenList tokenList;
            std::shared_ptr<std::vector<NameRank>> countries;
            std::shared_ptr<std::vector<NameRank>> regions;
            std::shared_ptr<std::vector<NameRank>> counties;
            std::shared_ptr<std::vector<NameRank>> localities;
            std::shared_ptr<std::vector<NameRank>> neighbourhoods;
            std::shared_ptr<std::vector<NameRank>> streets;
            std::shared_ptr<std::vector<NameRank>> postcodes;
            std::shared_ptr<std::vector<NameRank>> names;
            std::shared_ptr<std::string> houseNumber;
            
            int unmatchedTokens() const { return tokenList.unmatchedTokens(); }

            bool null() const {
                return !countries && !regions && !counties && !localities && !neighbourhoods && !streets && !postcodes && !names && !houseNumber;
            }
            
            float bestRank(FieldType type, std::size_t index) const {
                float rank = 1.0f;
                if (countries) {
                    rank *= (type != FieldType::COUNTRY ? (countries->empty() ? 0.0f : countries->front().rank) : countries->at(index).rank);
                }
                if (regions) {
                    rank *= (type != FieldType::REGION ? (regions->empty() ? 0.0f : regions->front().rank) : regions->at(index).rank);
                }
                if (counties) {
                    rank *= (type != FieldType::COUNTY ? (counties->empty() ? 0.0f : counties->front().rank) : counties->at(index).rank);
                }
                if (localities) {
                    rank *= (type != FieldType::LOCALITY ? (localities->empty() ? 0.0f : localities->front().rank) : localities->at(index).rank);
                }
                if (neighbourhoods) {
                    rank *= (type != FieldType::NEIGHBOURHOOD ? (neighbourhoods->empty() ? 0.0f : neighbourhoods->front().rank) : neighbourhoods->at(index).rank);
                }
                if (streets) {
                    rank *= (type != FieldType::STREET ? (streets->empty() ? 0.0f : streets->front().rank) : streets->at(index).rank);
                }
                if (postcodes) {
                    rank *= (type != FieldType::POSTCODE ? (postcodes->empty() ? 0.0f : postcodes->front().rank) : postcodes->at(index).rank);
                }
                if (names) {
                    rank *= (type != FieldType::NAME ? (names->empty() ? 0.0f : names->front().rank) : names->at(index).rank);
                }
                rank *= std::pow(UNMATCHED_FIELD_PENALTY, tokenList.unmatchedInvalidTokens());
                return rank;
            }
            
            static std::shared_ptr<std::vector<NameRank>> Query::* getField(FieldType type) {
                switch (type) {
                case FieldType::COUNTRY:
                    return &Query::countries;
                case FieldType::REGION:
                    return &Query::regions;
                case FieldType::COUNTY:
                    return &Query::counties;
                case FieldType::LOCALITY:
                    return &Query::localities;
                case FieldType::NEIGHBOURHOOD:
                    return &Query::neighbourhoods;
                case FieldType::STREET:
                    return &Query::streets;
                case FieldType::POSTCODE:
                    return &Query::postcodes;
                case FieldType::NAME:
                    return &Query::names;
                default:
                    throw std::invalid_argument("Illegal getField argument");
                }
            }
        };
        
        struct Result {
            std::shared_ptr<Database> database;
            std::uint64_t encodedId = 0;
            int unmatchedTokens = 0;
            float matchRank = 1.0f;
            float entityRank = 1.0f;
            float locationRank = 1.0f;

            float totalRank() const { return matchRank * entityRank * locationRank; }
        };

        void matchTokens(Query& query, int pass, TokenList& tokenList) const;
        void matchQuery(Query& query, const Options& options, std::set<std::vector<std::pair<FieldType, std::string>>>& assignments, std::vector<Result>& results) const;
        void matchNames(const Query& query, FieldType type, const std::vector<std::vector<Token>>& tokensList, const std::string& matchName, std::shared_ptr<std::vector<NameRank>>& nameRanks) const;
        void resolveQuery(const Query& query, const Options& options, std::vector<Result>& results) const;

        float calculateNameRank(const Query& query, const std::string& name, const std::string& queryName, const std::vector<std::pair<std::string, float>>& tokenIDFs) const;
        
        std::vector<std::string> buildQueryFilters(const Query& query) const;

        void updateMatchRank(const Database& database, FieldType type, const Query& query, std::uint64_t dbId, Result& result) const;
        void updateLocationRank(const Database& database, const Options& options, const std::vector<Feature>& features, Result& result) const;

        bool filterQuery(Query& query, const std::vector<Result>& results) const;
        bool bindQueryNameField(Query& query, FieldType type, std::shared_ptr<std::vector<NameRank>> Query::* field, const TokenList::Span& span) const;
        bool bindQueryStringField(Query& query, FieldType type, std::shared_ptr<std::string> Query::* field, const TokenList::Span& span) const;
        
        static cglib::vec2<double> getOrigin(sqlite3pp::database& db);
        static std::unordered_map<unichar_t, unistring> getTranslationTable(sqlite3pp::database& db);
        static unistring getTranslatedToken(const unistring& token, const std::unordered_map<unichar_t, unistring>& translationTable);

        static constexpr float ENTITY_BLOOM_FILTER_FP_PROB = 0.01f;
		static constexpr float ENTITY_BLOOM_FILTER_MAX_SIZE = 16384 * 1024;
        static constexpr float MIN_LOCATION_RANK = 0.2f; // should be larger than MIN_RANK
		static constexpr float MIN_RANK_THRESHOLD = 0.1f;
		static constexpr float MAX_RANK_RATIO = 0.5f;
        static constexpr float MIN_MATCH_THRESHOLD = 0.55f;
		static constexpr float MAX_MATCH_RATIO = 0.85f;
		static constexpr float MIN_IDF_THRESHOLD = 0.25f;
        static constexpr float EXTRA_FIELD_PENALTY = 0.9f;
        static constexpr float UNMATCHED_FIELD_PENALTY = 0.5f;
        static constexpr float POI_POPULATION_PENALTY = 0.99f;
        static constexpr float TRANSLATION_EXTRA_PENALTY = 0.3f;
        static constexpr float AUTOCOMPLETE_EXTRA_CHAR_PENALTY = 0.1f;
        static constexpr unsigned int MAX_STRINGMATCH_DIST = 2;
        static constexpr unsigned int MAX_RESULTS = 20;
        static constexpr std::size_t MIN_AUTOCOMPLETE_SIZE = 3;

        static constexpr std::size_t ADDRESS_CACHE_SIZE = 1024;
        static constexpr std::size_t ENTITY_CACHE_SIZE = 128;
        static constexpr std::size_t NAME_CACHE_SIZE = 128;
		static constexpr std::size_t TOKEN_CACHE_SIZE = 128;
		static constexpr std::size_t NAME_RANK_CACHE_SIZE = 128;
        static constexpr std::size_t NAME_MATCH_CACHE_SIZE = 4096;
        
        bool _autocomplete = false; // no autocomplete by default
        std::string _language; // use local language by default
        std::vector<Address::Type> _enabledFilters; // filters enabled, empty list means 'all enabled'

        mutable cache::lru_cache<std::string, Address> _addressCache;
        mutable cache::lru_cache<std::string, std::vector<EntityRow>> _entityCache;
        mutable cache::lru_cache<std::string, std::vector<std::shared_ptr<Name>>> _nameCache;
		mutable cache::lru_cache<std::string, std::vector<Token>> _tokenCache;
		mutable cache::lru_cache<std::string, std::shared_ptr<std::vector<NameRank>>> _nameRankCache;
        mutable cache::lru_cache<std::string, float> _nameMatchCache;
        mutable std::uint64_t _addressQueryCounter = 0;
        mutable std::uint64_t _entityQueryCounter = 0;
        mutable std::uint64_t _missingEntityQueryCounter = 0;
        mutable std::uint64_t _nameQueryCounter = 0;
		mutable std::uint64_t _tokenQueryCounter = 0;
        mutable std::uint64_t _nameRankCounter = 0;
        mutable std::uint64_t _nameMatchCounter = 0;
		mutable std::uint64_t _bloomTestCounter = 0;

        mutable std::vector<std::shared_ptr<Database>> _databases;
        mutable std::recursive_mutex _mutex;
    };
} }

#endif
