/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_REVGEOCODER_H_
#define _CARTO_GEOCODING_REVGEOCODER_H_

#include "Address.h"
#include "Geometry.h"
#include "QuadIndex.h"

#include <vector>
#include <memory>
#include <mutex>

#include <boost/optional.hpp>

#include <stdext/lru_cache.h>

#include <cglib/bbox.h>

namespace sqlite3pp {
    class database;
}

namespace carto { namespace geocoding {
    class RevGeocoder final {
    public:
        RevGeocoder() : _addressCache(ADDRESS_CACHE_SIZE), _queryCache(QUERY_CACHE_SIZE) { }
        
        bool import(const std::shared_ptr<sqlite3pp::database>& db);

        std::string getLanguage() const;
        void setLanguage(const std::string& language);

        unsigned int getMaxResults() const;
        void setMaxResults(unsigned int maxResults);

        bool isFilterEnabled(Address::EntityType type) const;
        void setFilterEnabled(Address::EntityType type, bool enabled);

        std::vector<std::pair<Address, float>> findAddresses(double lng, double lat, float radius) const;

    private:
        struct Database {
            std::string id;
            std::shared_ptr<sqlite3pp::database> db;
            cglib::vec2<double> origin;
            boost::optional<cglib::bbox2<double>> bounds;
        };
        
        std::vector<QuadIndex::GeometryInfo> findGeometryInfo(const Database& database, const std::vector<std::uint64_t>& quadIndices, const PointConverter& converter) const;

        static cglib::vec2<double> getOrigin(sqlite3pp::database& db);
        static boost::optional<cglib::bbox2<double>> getBounds(sqlite3pp::database& db);

        static constexpr std::size_t ADDRESS_CACHE_SIZE = 1024;
        static constexpr std::size_t QUERY_CACHE_SIZE = 64;
        
        std::string _language; // use local language by default
        unsigned int _maxResults = 10; // maximum number of results returned
        std::vector<Address::EntityType> _enabledFilters = { Address::EntityType::ADDRESS, Address::EntityType::POI }; // filters enabled

        mutable cache::lru_cache<std::string, Address> _addressCache;
        mutable cache::lru_cache<std::string, std::vector<QuadIndex::GeometryInfo>> _queryCache;
        mutable std::uint64_t _previousEntityQueryCounter = 0;;
        mutable std::uint64_t _entityQueryCounter = 0;

        std::vector<Database> _databases;
        mutable std::recursive_mutex _mutex;
    };
} }

#endif
