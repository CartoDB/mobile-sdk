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
        
        explicit Geocoder(sqlite3pp::database& db) : _addressCache(ADDRESS_CACHE_SIZE), _populationCache(POPULATION_CACHE_SIZE), _tokenIDFCache(TOKEN_IDF_CACHE_SIZE), _nameRankCache(NAME_RANK_CACHE_SIZE), _nameQueryCache(NAME_QUERY_CACHE_SIZE), _nameMatchCache(NAME_MATCH_CACHE_SIZE), _db(db) { _origin = getOrigin(); _translationTable = getTranslationTable(); }

        bool getAutocomplete() const;
        void setAutocomplete(bool autocomplete);

        std::string getLanguage() const;
        void setLanguage(const std::string& language);

        bool isFilterEnabled(Address::Type type) const;
        void setFilterEnabled(Address::Type type, bool enabled);
        
        std::vector<std::pair<Address, float>> findAddresses(const std::string& queryString, const Options& options) const;

    private:
        enum class TokenType {
            NONE, COUNTRY, REGION, COUNTY, LOCALITY, NEIGHBOURHOOD, STREET, POSTCODE, NAME, HOUSENUMBER
        };

        struct Token {
            std::uint64_t id = 0;
            float idf = 0.0f;
        };
        
        struct Name {
            std::uint64_t id = 0;
            std::string name;
            std::string lang;
            std::uint64_t xmask = 0;
            std::uint64_t ymask = 0;
            float idf = 1.0f;
        };

        struct Ranking {
            float matchRank = 1.0f;
            float populationRank = 1.0f;
            float locationRank = 1.0f;
            float rank() const { return matchRank * populationRank * locationRank; }
        };
        
        using TokenListType = TokenList<std::string, TokenType, std::vector<Token>>;

        struct Query {
            Options options;
            TokenListType tokenList;
            std::vector<Name> countries;
            std::vector<Name> regions;
            std::vector<Name> counties;
            std::vector<Name> localities;
            std::vector<Name> neighbourhoods;
            std::vector<Name> streets;
            std::vector<Name> postcodes;
            std::vector<Name> names;
            std::string houseNumber;
            int unmatchedTokens() const { return tokenList.unmatchedTokens(); }
        };
        
        struct Result {
            std::uint64_t encodedId = 0;
            int unmatchedTokens = 0;
            Ranking ranking;
        };

        void classifyTokens(TokenListType& tokenList) const;
        void matchTokens(const Query& query, std::vector<Query>& candidates) const;
        void resolveAddress(const Query& query, std::vector<Result>& results) const;

        std::vector<Name> matchTokenNames(TokenType type, const std::vector<std::vector<Token>>& tokensList, const std::vector<std::string>& tokenStrings) const;
        
        std::vector<std::string> buildQueryFilters(const Query& query) const;
        
        bool optimizeQuery(Query& query) const;
        bool bindQueryNameField(Query& query, TokenType type, std::vector<Name> Query::* field, const TokenListType::Span& span) const;
        bool bindQueryStringField(Query& query, TokenType type, std::string Query::* field, const TokenListType::Span& span) const;
        
        float calculateNameRank(const std::string& name, const std::string& queryName) const;
        float calculateMatchRank(TokenType type, const std::vector<Name>& matchNames, std::uint64_t dbId, const TokenListType& tokenList) const;
        float calculatePopulationRank(TokenType type, std::uint64_t id) const;

        float getTokenRank(const unistring& unitoken) const;
        cglib::vec2<double> getOrigin() const;
        std::unordered_map<unichar_t, unistring> getTranslationTable() const;

        static constexpr float MIN_RANK = 0.1f;
        static constexpr float MIN_LOCATION_RANK = 0.2f; // should be larger than MIN_RANK
        static constexpr float MAX_RANK_RATIO = 0.5f;
        static constexpr float MIN_IDF_THRESHOLD = 0.5f;
        static constexpr float EXTRA_FIELD_PENALTY = 0.95f;
        static constexpr float UNMATCHED_FIELD_PENALTY = 0.5f;
        static constexpr float POI_POPULATION_PENALTY = 0.9f;
        static constexpr float TRANSLATION_EXTRA_PENALTY = 0.1f;
        static constexpr float AUTOCOMPLETE_EXTRA_CHAR_PENALTY = 0.1f;
        static constexpr unsigned int MAX_STRINGMATCH_DIST = 2;
        static constexpr unsigned int MAX_ADDRESS_LOOKUPS = 50;
        static constexpr unsigned int MAX_RESULTS = 50;
        static constexpr std::size_t MIN_AUTOCOMPLETE_SIZE = 3;
        static constexpr std::size_t ADDRESS_CACHE_SIZE = 1024;
        static constexpr std::size_t POPULATION_CACHE_SIZE = 1024;
        static constexpr std::size_t TOKEN_IDF_CACHE_SIZE = 1024;
        static constexpr std::size_t NAME_RANK_CACHE_SIZE = 1024;
        static constexpr std::size_t NAME_QUERY_CACHE_SIZE = 128;
        static constexpr std::size_t NAME_MATCH_CACHE_SIZE = 128;

        bool _autocomplete = false; // no autocomplete by default
        std::string _language; // use local language by default
        std::vector<Address::Type> _enabledFilters = { }; // filters enabled, empty list means 'all enabled'

        mutable cache::lru_cache<std::uint64_t, Address> _addressCache;
        mutable cache::lru_cache<std::string, std::uint64_t> _populationCache;
        mutable cache::lru_cache<std::string, float> _tokenIDFCache;
        mutable cache::lru_cache<std::string, float> _nameRankCache;
        mutable cache::lru_cache<std::string, std::vector<Name>> _nameQueryCache;
        mutable cache::lru_cache<std::string, std::vector<Name>> _nameMatchCache;
        mutable std::uint64_t _entityQueryCounter = 0;
        mutable std::uint64_t _populationQueryCounter = 0;
        mutable std::uint64_t _tokenIDFQueryCounter = 0;
        mutable std::uint64_t _nameRankCounter = 0;
        mutable std::uint64_t _nameQueryCounter = 0;
        mutable std::uint64_t _matchTokensCounter = 0;

        cglib::vec2<double> _origin;
        std::unordered_map<unichar_t, unistring> _translationTable;
        sqlite3pp::database& _db;
        mutable std::recursive_mutex _mutex;
    };
} }

#endif
