/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_FEATURECOLLECTION_H_
#define _CARTO_MAPNIKVT_FEATURECOLLECTION_H_

#include "Feature.h"

#include <vector>
#include <utility>

namespace carto { namespace mvt {
    class FeatureCollection final {
    public:
        FeatureCollection() = default;

        void clear() {
            _features.clear();
        }
        
        void append(long long localId, const Feature& feature) {
            _features.emplace_back(localId, feature);
        }

        std::size_t size() const {
            return _features.size();
        }

        long long getLocalId(std::size_t index) const { return _features.at(index).first; }
        long long getGlobalId(std::size_t index) const { return _features.at(index).second.getId(); }

        const Feature& getFeature(std::size_t index) const { return _features.at(index).second; }
        const std::shared_ptr<const Geometry>& getGeometry(std::size_t index) const { return _features.at(index).second.getGeometry(); }
        const std::shared_ptr<const FeatureData>& getFeatureData(std::size_t index) const { return _features.at(index).second.getFeatureData(); }

    private:
        std::vector<std::pair<long long, Feature>> _features;
    };
} }

#endif
