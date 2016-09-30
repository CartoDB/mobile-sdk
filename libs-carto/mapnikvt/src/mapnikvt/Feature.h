/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_FEATURE_H_
#define _CARTO_MAPNIKVT_FEATURE_H_

#include "Geometry.h"
#include "FeatureData.h"

#include <memory>
#include <list>
#include <vector>
#include <map>

namespace carto { namespace mvt {
    class Feature final {
    public:
        Feature(long long id, std::shared_ptr<const Geometry> geometry, std::shared_ptr<const FeatureData> featureData) : _id(id), _geometry(geometry), _featureData(featureData) { }

        long long getId() const { return _id; }
        const std::shared_ptr<const Geometry>& getGeometry() const { return _geometry; }
        const std::shared_ptr<const FeatureData>& getFeatureData() const { return _featureData; }

    private:
        long long _id;
        std::shared_ptr<const Geometry> _geometry;
        std::shared_ptr<const FeatureData> _featureData;
    };
} }

#endif
