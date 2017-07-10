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

        bool isFilterEnabled(Address::EntityType type) const;
        void setFilterEnabled(Address::EntityType type, bool enabled);
        
        std::vector<std::pair<Address, float>> findAddresses(const std::string& queryString, const Options& options) const;

    private:
        using FieldType = Address::FieldType;
        
        struct Token {
            std::uint64_t id = 0;
            std::uint64_t count = 0;
            std::string token;
            std::uint32_t typeMask = 0;
            float idf = 1.0f;
        };
        
        using TokenList = TaggedTokenList<std::string, FieldType, std::vector<Token>>;

        struct Name {
            std::uint64_t id = 0;
            std::uint64_t count = 0;
            std::string name;
            std::string lang;
            FieldType type = FieldType::NONE;
            std::vector<std::pair<std::string, float>> tokenIDFs;
        };

        struct NameRank {
            std::shared_ptr<Name> name;
            float rank;
        };

        struct EntityName {
            std::uint64_t id = 0;
            FieldType type = FieldType::NONE;
        };

        struct EntityRow {
            std::uint64_t id = 0;
            std::string features;
            std::string houseNumbers;
            std::vector<EntityName> entityNames;
            float rank = 0.0f;
        };

        struct Database {
            std::string id;
            std::shared_ptr<sqlite3pp::database> db;
            cglib::vec2<double> origin = cglib::vec2<double>(0, 0);
            double rankScale = 1.0;
            std::unordered_map<unichar_t, unistring> translationTable;
        };

        struct Query {
            std::shared_ptr<Database> database;
            TokenList tokenList;
            std::vector<std::shared_ptr<std::vector<NameRank>>> filtersList;
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
        void matchQuery(Query& query, const Options& options, std::set<std::vector<std::pair<std::uint32_t, std::string>>>& assignments, std::vector<Result>& results) const;
        void matchNames(const Query& query, const std::vector<std::vector<Token>>& tokensList, const std::string& matchName, std::shared_ptr<std::vector<NameRank>>& nameRanks) const;
        void matchEntities(const Query& query, const Options& options, std::vector<Result>& results) const;

        bool optimizeQuery(Query& query) const;

        float calculateNameRank(const Query& query, const std::string& name, const std::string& queryName, const std::vector<std::pair<std::string, float>>& tokenIDFs) const;
        
        static cglib::vec2<double> getOrigin(sqlite3pp::database& db);
        static std::unordered_map<unichar_t, unistring> getTranslationTable(sqlite3pp::database& db);
        static double getRankScale(sqlite3pp::database& db);
        static unistring getTranslatedToken(const unistring& token, const std::unordered_map<unichar_t, unistring>& translationTable);

        static constexpr float MIN_LOCATION_RANK = 0.2f; // should be larger than MIN_RANK
        static constexpr float MIN_RANK_THRESHOLD = 0.1f;
        static constexpr float MAX_RANK_RATIO = 0.5f;
        static constexpr float MIN_MATCH_THRESHOLD = 0.55f;
        static constexpr float MAX_MATCH_RATIO = 0.85f;
        static constexpr float EXTRA_FIELD_PENALTY = 0.9f;
        static constexpr float UNMATCHED_FIELD_PENALTY = 0.5f;
        static constexpr float POI_POPULATION_PENALTY = 0.99f;
        static constexpr float TRANSLATION_EXTRA_PENALTY = 0.3f;
        static constexpr float AUTOCOMPLETE_EXTRA_CHAR_PENALTY = 0.1f;
        static constexpr unsigned int MAX_STRINGMATCH_DIST = 2;
        static constexpr unsigned int MAX_RESULTS = 20;
        static constexpr std::size_t MIN_AUTOCOMPLETE_SIZE = 3;
        static constexpr std::size_t MAX_MATCH_COUNT = 10000;
        static constexpr std::size_t MAX_ENTITY_MATCH_COUNTER = 10000;
        static constexpr std::size_t MAX_NAME_MATCH_COUNTER = 1000;

        static constexpr std::size_t ADDRESS_CACHE_SIZE = 1024;
        static constexpr std::size_t ENTITY_CACHE_SIZE = 128;
        static constexpr std::size_t NAME_CACHE_SIZE = 128;
        static constexpr std::size_t TOKEN_CACHE_SIZE = 128;
        static constexpr std::size_t NAME_RANK_CACHE_SIZE = 128;
        static constexpr std::size_t NAME_MATCH_CACHE_SIZE = 4096;
        
        bool _autocomplete = false; // no autocomplete by default
        std::string _language; // use local language by default
        std::vector<Address::EntityType> _enabledFilters; // filters enabled, empty list means 'all enabled'

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

        mutable std::vector<std::shared_ptr<Database>> _databases;
        mutable std::recursive_mutex _mutex;
    };
} }

#endif
