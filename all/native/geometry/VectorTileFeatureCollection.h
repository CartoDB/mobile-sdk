/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORTILEFEATURECOLLECTION_H_
#define _CARTO_VECTORTILEFEATURECOLLECTION_H_

#include "geometry/FeatureCollection.h"
#include "geometry/VectorTileFeature.h"

#include <memory>
#include <vector>

namespace carto {

    /**
     * A collection of vector tile features.
     */
    class VectorTileFeatureCollection : public FeatureCollection {
    public:
        /**
         * Constructs a VectorTileFeatureCollection from a vector of vector tile features.
         * @param features The features for the collection.
         */
        explicit VectorTileFeatureCollection(const std::vector<std::shared_ptr<VectorTileFeature> >& features);
        virtual ~VectorTileFeatureCollection();

        /**
         * Returns the feature at the specified index. Index must be between 0 and getFeatureCount (exclusive).
         * @return The feature at specified index.
         * @throws std::out_of_range If the index is out of range.
         */
        std::shared_ptr<VectorTileFeature> getFeature(int index) const;
    };

}

#endif
