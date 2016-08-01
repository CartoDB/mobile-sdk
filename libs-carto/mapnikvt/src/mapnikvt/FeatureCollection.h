/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_FEATURECOLLECTION_H_
#define _CARTO_MAPNIKVT_FEATURECOLLECTION_H_

#include "Value.h"
#include "Geometry.h"
#include "FeatureData.h"

#include <memory>
#include <list>
#include <vector>
#include <map>

#include <cglib/vec.h>

namespace carto { namespace mvt {
    class FeatureCollection {
    public:
        FeatureCollection() = default;

        void clear() {
            _ids.clear();
            _geometries.clear();
        }
        
        void append(long long id, std::shared_ptr<Geometry> geometry) {
            _ids.push_back(id);
            _geometries.push_back(std::move(geometry));
        }

        void setFeatureData(std::shared_ptr<FeatureData> featureData) {
            _featureData = std::move(featureData);
        }

        std::size_t getSize() const { return _ids.size(); }
        long long getId(std::size_t index) const { return _ids.at(index); }
        const std::shared_ptr<Geometry>& getGeometry(std::size_t index) const { return _geometries.at(index); }
        const std::shared_ptr<FeatureData>& getFeatureData() const { return _featureData; }

    private:
        std::vector<long long> _ids;
        std::vector<std::shared_ptr<Geometry>> _geometries;
        std::shared_ptr<FeatureData> _featureData;
    };
} }

#endif
