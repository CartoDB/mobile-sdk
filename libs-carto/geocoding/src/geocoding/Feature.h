/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_FEATURE_H_
#define _CARTO_GEOCODING_FEATURE_H_

#include "Geometry.h"

#include <cstdint>
#include <memory>
#include <map>

#include <boost/variant.hpp>

namespace carto { namespace geocoding {
    using Value = boost::variant<boost::blank, bool, long long, double, std::string>;

    class Feature final {
    public:
        explicit Feature(std::uint64_t id, std::shared_ptr<Geometry> geom, std::map<std::string, Value> properties) : _id(id), _geometry(std::move(geom)), _properties(std::move(properties)) { }

        std::uint64_t getId() const { return _id; }
        const std::shared_ptr<Geometry>& getGeometry() const { return _geometry; }
        const std::map<std::string, Value>& getProperties() const { return _properties; }

    private:
        std::uint64_t _id;
        std::shared_ptr<Geometry> _geometry;
        std::map<std::string, Value> _properties;
    };
} }

#endif
