/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_FEATURECOLLECTION_H_
#define _CARTO_MAPNIKVT_FEATURECOLLECTION_H_

#include "Geometry.h"
#include "FeatureData.h"

#include <memory>
#include <tuple>
#include <vector>
#include <map>

namespace carto { namespace mvt {
    class FeatureCollection {
    public:
        FeatureCollection() = default;

        void clear() {
            _features.clear();
        }
        
        void append(long long localId, long long globalId, std::shared_ptr<const Geometry> geometry) {
            _features.emplace_back(localId, globalId, std::move(geometry));
        }

        void setFeatureData(std::shared_ptr<const FeatureData> featureData) {
            _featureData = std::move(featureData);
        }

        std::size_t getSize() const { return _features.size(); }
        long long getLocalId(std::size_t index) const { return std::get<0>(_features.at(index)); }
        long long getGlobalId(std::size_t index) const { return std::get<1>(_features.at(index)); }
        const std::shared_ptr<const Geometry>& getGeometry(std::size_t index) const { return std::get<2>(_features.at(index)); }
        const std::shared_ptr<const FeatureData>& getFeatureData() const { return _featureData; }

    private:
        std::vector<std::tuple<long long, long long, std::shared_ptr<const Geometry>>> _features;
        std::shared_ptr<const FeatureData> _featureData;
    };
} }

#endif
