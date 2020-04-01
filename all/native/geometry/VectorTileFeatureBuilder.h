/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORTILEFEATUREBUILDER_H_
#define _CARTO_VECTORTILEFEATUREBUILDER_H_

#include "core/MapTile.h"
#include "geometry/FeatureBuilder.h"

#include <string>
#include <memory>

namespace carto {
    class VectorTileFeature;

    /**
     * A vector tile feature consisting of id, layer, geometry and properties.
     */
    class VectorTileFeatureBuilder : public FeatureBuilder {
    public:
        /**
         * Constructs a VectorTileFeatureBuilder with empty state.
         */
        VectorTileFeatureBuilder();
        virtual ~VectorTileFeatureBuilder();

        /**
         * Returns the id of the builder.
         * @return The id of the builder.
         */
        long long getId() const;
        /**
         * Sets the id of the builder.
         * @param id The id of the builder.
         */
        void setId(long long id);

        /**
         * Returns the map tile of the builder.
         * @return The map tile of the builder.
         */
        MapTile getMapTile() const;
        /**
         * Sets the map tile of the builder.
         * @param mapTile The map tile of the builder.
         */
        void setMapTile(const MapTile& mapTile);

        /**
         * Returns the layer name of the builder.
         * @return The layer name of the builder.
         */
        std::string getLayerName() const;
        /**
         * Sets the layer name of the builder.
         * @param layerName The layer name of the builder.
         */
        void setLayerName(const std::string& layerName);

        /**
         * Builds a new vector tile feature from the state.
         * @return A new vector tile feature based on the builder state. Can be null if the geometry is not specified.
         */
        std::shared_ptr<VectorTileFeature> buildVectorTileFeature() const;
        
    protected:
        long long _id;
        MapTile _mapTile;
        std::string _layerName;
    };
    
}

#endif
