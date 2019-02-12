/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SPATIALINDEX_H_
#define _CARTO_SPATIALINDEX_H_

#include <vector>

#include <cglib/vec.h>
#include <cglib/bbox.h>
#include <cglib/frustum3.h>

namespace carto {
    
    template <typename T>
    class SpatialIndex {
    public:
        virtual ~SpatialIndex() { }
        
        virtual std::size_t size() const = 0;
        virtual void reserve(std::size_t size) = 0;
        
        virtual void clear() = 0;
        virtual void insert(const cglib::bbox3<double>& bounds, const T& object) = 0;
        virtual bool remove(const cglib::bbox3<double>& bounds, const T& object) = 0;
        virtual bool remove(const T& object) = 0;
        
        virtual std::vector<T> query(const cglib::frustum3<double>& frustum) const = 0;
        virtual std::vector<T> query(const cglib::bbox3<double>& bounds) const = 0;
        virtual std::vector<T> getAll() const = 0;
    };

}

#endif
