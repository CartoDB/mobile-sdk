/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORTILEFEATURE_H_
#define _CARTO_VECTORTILEFEATURE_H_

#include "core/MapTile.h"
#include "geometry/Feature.h"

#include <string>
#include <memory>

namespace carto {

    /**
     * A vector tile feature consisting of id, layer, geometry and properties.
     */
    class VectorTileFeature : public Feature {
    public:
        /**
         * Constructs a VectorTileFeature object from id, layer name, geometry and properties.
         * @param id The id of the feature.
         * @param mapTile The map tile of this feature
         * @param layerName The name of the layer of the feature.
         * @param geometry The geometry of the feature.
         * @param properties The properties of the feature.
         */
        VectorTileFeature(long long id, const MapTile& mapTile, const std::string& layerName, const std::shared_ptr<Geometry>& geometry, Variant properties);
        virtual ~VectorTileFeature();

        /**
         * Returns the id of the feature.
         * @return The id of the feature.
         */
        long long getId() const;

        /**
         * Returns the map tile of the feature.
         * @return The map tile of the feature.
         */
        const MapTile& getMapTile() const;

        /**
         * Returns the layer name of the feature.
         * @return The layer name of the feature.
         */
        const std::string& getLayerName() const;
        
    protected:
        const long long _id;
        const MapTile _mapTile;
        const std::string _layerName;
    };
    
}

#endif
