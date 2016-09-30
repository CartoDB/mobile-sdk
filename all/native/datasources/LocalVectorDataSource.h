/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LOCALVECTORDATASOURCE_H_
#define _CARTO_LOCALVECTORDATASOURCE_H_

#include "core/MapBounds.h"
#include "datasources/VectorDataSource.h"
#include "geometry/utils/SpatialIndex.h"

#include <memory>

namespace carto {

    namespace LocalSpatialIndexType {
        /**
         * Spatial index type for local data source.
         */
        enum LocalSpatialIndexType {
            /**
             * Null index, fastest if few elements are used. No element culling is performed.
             */
            LOCAL_SPATIAL_INDEX_TYPE_NULL,
            
            /**
             * K-d tree index, element culling is exact and fast.
             */
            LOCAL_SPATIAL_INDEX_TYPE_KDTREE
        };
    }

    class GeometrySimplifier;

    /**
     * A modifiable vector data source that keeps all the elements in the local memory.
     * Optionally vector elements can be kept in a spatial index and only the visible elements get rendered.
     * There can be a small delay before previously invisible elements become visible after view changes.
     * This makes it suitable for cases where there are a large number of static vector elements.
     *
     * The draw order of vector elements within the data source is undefined.
     */
    class LocalVectorDataSource: public VectorDataSource {
    public:
        /**
         * Constructs an LocalVectorDataSource object with no spatial index.
         * @param projection The projection used by this data source.
         */
        explicit LocalVectorDataSource(const std::shared_ptr<Projection>& projection);
        /**
         * Constructs an LocalVectorDataSource object with specified spatial index.
         * @param projection The projection used by this data source.
         * @param spatialIndexType The spatial index type to use.
         */
        LocalVectorDataSource(const std::shared_ptr<Projection>& projection, LocalSpatialIndexType::LocalSpatialIndexType spatialIndexType);
        virtual ~LocalVectorDataSource();
        
        virtual std::shared_ptr<VectorData> loadElements(const std::shared_ptr<CullState>& cullState);

        /**
         * Clear the data source by removing all elements.
         */
        void clear();
        
        /**
         * Returns all vector elements added to this data source using add() and addAll() methods.
         * @return A vector containing all the vector elements in this data source.
         */
        std::vector<std::shared_ptr<VectorElement> > getAll() const;
        
        /**
         * Replaces all vector elements in the data source with the given list of vector elements.
         * @param elements The list of new vector elements
         */
        void setAll(const std::vector<std::shared_ptr<VectorElement> >& elements);
        
        /**
         * Adds a vector element to the data source. The vector element's coordinates are expected to be
         * in the data source's projection's coordinate system. The order in which the elements are added has
         * no effect on the order they get rendered.
         * @param element The vector element to be added.
         */
        void add(const std::shared_ptr<VectorElement>& element);
        /**
         * Adds multiple vector elements to the data source. The vector elements' coordinates are expected to be
         * in the data source's projection's coordinate system. The order in which the elements are added has
         * no effect on the order they get rendered.
         * @param elements A list of vector elements to be added.
         */
        void addAll(const std::vector<std::shared_ptr<VectorElement> >& elements);

        /**
         * Removes a vector element from the data source.
         * @param element The vector element to be removed.
         * @return True if the vector element existed in the data source.
         */
        bool remove(const std::shared_ptr<VectorElement>& element);
        /**
         * Removes multiple vector elements from the data source.
         * @param elements A list of vector elements to be removed.
         * @return True if all listed elements were removed. False otherwise.
         */
        bool removeAll(const std::vector<std::shared_ptr<VectorElement> >& elements);
        
        /**
         * Returns the active geometry simplifier of the data source.
         * @return The current geometry simplifier (can be null)
         */
        std::shared_ptr<GeometrySimplifier> getGeometrySimplifier() const;
        /**
         * Attaches geometry simplifier to the data source. If the specified simplifier is null, detaches any active simplifier.
         * @param simplifier The new geometry simplifier to use (can be null).
         */
        void setGeometrySimplifier(const std::shared_ptr<GeometrySimplifier>& simplifier);
        
        /**
         * Returns the extent of this data source. Extent is the minimal bounding box encompassing all the elements.
         * @return The minimal bounding box for the elements.
         */
        MapBounds getDataExtent() const;
        
    protected:
        std::shared_ptr<VectorElement> simplifyElement(const std::shared_ptr<VectorElement>& element, float scale) const;

        virtual void notifyElementChanged(const std::shared_ptr<VectorElement>& element);

    private:
        std::shared_ptr<GeometrySimplifier> _geometrySimplifier;
        std::shared_ptr<SpatialIndex<std::shared_ptr<VectorElement> > > _spatialIndex;
        
        unsigned int _elementId;

        mutable std::mutex _mutex;
    };
    
}

#endif
