/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SPATIALINDEX_H_
#define _CARTO_SPATIALINDEX_H_

#include "core/MapBounds.h"
#include "core/MapPos.h"
#include "core/MapVec.h"
#include "graphics/Frustum.h"

#include <vector>

namespace carto {
    
    template <typename T>
    class SpatialIndex {
    public:
        virtual ~SpatialIndex() = default;
        
        virtual std::size_t size() const = 0;
        
        virtual void clear() = 0;
        virtual void insert(const MapBounds& bounds, const T& object) = 0;
        virtual bool remove(const MapBounds& bounds, const T& object) = 0;
        virtual bool remove(const T& object) = 0;
        
        virtual std::vector<T> query(const Frustum& frustum) const = 0;
        virtual std::vector<T> query(const MapBounds& bounds) const = 0;
        virtual std::vector<T> getAll() const = 0;
    };

}

#endif
