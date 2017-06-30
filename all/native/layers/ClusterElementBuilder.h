/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CLUSTERELEMENTBUILDER_H_
#define _CARTO_CLUSTERELEMENTBUILDER_H_

#include "core/MapPos.h"

#include <vector>
#include <memory>

namespace carto {
    class VectorElement;

    namespace ClusterBuilderMode {
        /**
         * Possible cluster builder modes.
         */
        enum ClusterBuilderMode {
            /**
             * Cluster builder receives full list of elements in the cluster. This mode is more expensive compared to the light mode.
             */
            CLUSTER_BUILDER_MODE_ELEMENTS,

            /**
             * Cluster builder receives element count in the cluster. This mode is less expensive compared to the full mode.
             */
            CLUSTER_BUILDER_MODE_ELEMENT_COUNT
        };
    }

    /**
     * A special callback interface for building cluster elements from set of existing vector element.
     */
    class ClusterElementBuilder {
    public:
        virtual ~ClusterElementBuilder() { }

        /**
         * Returns the builder mode. The return value of this method determines which builder method is called.
         * @return The builder mode that determines which buildClusterElement method is called.
         */
        virtual ClusterBuilderMode::ClusterBuilderMode getBuilderMode() const { return ClusterBuilderMode::CLUSTER_BUILDER_MODE_ELEMENTS; }

        /**
         * A callback for building a new cluster element given position and the number of cluster element.
         * @param mapPos The position of the cluster element.
         * @param elementCount The number of elements in the cluster.
         * @return The created cluster element.
         */
        virtual std::shared_ptr<VectorElement> buildClusterElement(const MapPos& mapPos, int elementCount) const { return std::shared_ptr<VectorElement>(); }

        /**
         * A callback for building a new cluster element given position and list of vector elements.
         * @param mapPos The position of the cluster element.
         * @param elements The list of elements that defines the cluster.
         * @return The created cluster element.
         */
        virtual std::shared_ptr<VectorElement> buildClusterElement(const MapPos& mapPos, const std::vector<std::shared_ptr<VectorElement> >& elements) const { return std::shared_ptr<VectorElement>(); }
    };
}

#endif
