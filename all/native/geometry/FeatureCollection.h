/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FEATURECOLLECTION_H_
#define _CARTO_FEATURECOLLECTION_H_

#include "geometry/Feature.h"

#include <memory>
#include <vector>

namespace carto {

    /**
     * A collection of features.
     */
    class FeatureCollection {
    public:
        /**
         * Constructs a FeatureCollection from a vector of features.
         * @param features The features for the collection.
         */
        explicit FeatureCollection(const std::vector<std::shared_ptr<Feature> >& features);
        virtual ~FeatureCollection();

        /**
         * Returns the number of features in this container.
         * @return The number of features.
         */
        int getFeatureCount() const;

        /**
         * Returns the feature at the specified index. Index must be between 0 and getFeatureCount (exclusive).
         * @return The feature at specified index.
         * @throws std::out_of_range If the index is out of range.
         */
        std::shared_ptr<Feature> getFeature(int index) const;

    protected:
        std::vector<std::shared_ptr<Feature> > _features;
    };

}

#endif
