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
        explicit RevGeocoder(sqlite3pp::database& db) : _addressCache(ADDRESS_CACHE_SIZE), _queryCache(QUERY_CACHE_SIZE), _db(db) { _bounds = findBounds(); _origin = findOrigin(); }

        float getRadius() const;
        void setRadius(float radius);

        std::string getLanguage() const;
        void setLanguage(const std::string& language);

        std::vector<std::pair<Address, float>> findAddresses(double lng, double lat) const;

    private:
        cglib::vec2<double> findOrigin() const;
        boost::optional<cglib::bbox2<double>> findBounds() const;

        std::vector<QuadIndex::GeometryInfo> findGeometryInfo(const std::vector<std::uint64_t>& quadIndices, const PointConverter& converter) const;

        static constexpr std::size_t ADDRESS_CACHE_SIZE = 1024;
        static constexpr std::size_t QUERY_CACHE_SIZE = 64;
        
        float _radius = 100.0f; // default search radius is 100m
        std::string _language; // use local language by default

        mutable cache::lru_cache<std::uint64_t, Address> _addressCache;
        mutable cache::lru_cache<std::string, std::vector<QuadIndex::GeometryInfo>> _queryCache;
        mutable std::uint64_t _previousEntityQueryCounter = 0;;
        mutable std::uint64_t _entityQueryCounter = 0;

        cglib::vec2<double> _origin;
        boost::optional<cglib::bbox2<double>> _bounds;
        sqlite3pp::database& _db;
        mutable std::recursive_mutex _mutex;
    };
} }

#endif
