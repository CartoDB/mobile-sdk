/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NULLSPATIALINDEX_H_
#define _CARTO_NULLSPATIALINDEX_H_

#include "geometry/utils/SpatialIndex.h"

#include <utility>

namespace carto {
    
    template <typename T>
    class NullSpatialIndex : public SpatialIndex<T> {
    public:
        NullSpatialIndex();
        
        virtual std::size_t size() const;
        
        virtual void clear();
        virtual void insert(const MapBounds& bounds, const T& object);
        virtual bool remove(const MapBounds& bounds, const T& object);
        virtual bool remove(const T& object);
        
        virtual std::vector<T> query(const Frustum& frustum) const;
        virtual std::vector<T> query(const MapBounds& bounds) const;
        virtual std::vector<T> getAll() const;
        
    private:
        std::vector<T> _objects;
    };
    
    template<typename T>
    NullSpatialIndex<T>::NullSpatialIndex() :
        _objects()
    {
    }
    
    template<typename T>
    std::size_t NullSpatialIndex<T>::size() const {
        return _objects.size();
    }
    
    template<typename T>
    void NullSpatialIndex<T>::clear() {
        _objects.clear();
    }
    
    template<typename T>
    void NullSpatialIndex<T>::insert(const MapBounds& bounds, const T& object) {
        _objects.push_back(object);
    }
    
    template<typename T>
    bool NullSpatialIndex<T>::remove(const MapBounds& bounds, const T& object) {
        return remove(object);
    }
    
    template<typename T>
    bool NullSpatialIndex<T>::remove(const T& object) {
        std::size_t count = _objects.size();
        while (true) {
            auto it = std::find(_objects.begin(), _objects.end(), object);
            if (it == _objects.end()) {
                break;
            }
            _objects.erase(it);
        }
        return count != _objects.size();
    }
    
    template<typename T>
    std::vector<T> NullSpatialIndex<T>::query(const Frustum& frustum) const {
        return _objects;
    }
    
    template<typename T>
    std::vector<T> NullSpatialIndex<T>::query(const MapBounds& bounds) const {
        return _objects;
    }
    
    template<typename T>
    std::vector<T> NullSpatialIndex<T>::getAll() const {
        return _objects;
    }
}

#endif
