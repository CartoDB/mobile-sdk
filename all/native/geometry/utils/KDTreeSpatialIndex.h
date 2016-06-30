/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_KDTREESPATIALINDEX_H_
#define _CARTO_KDTREESPATIALINDEX_H_

#include "geometry/utils/SpatialIndex.h"

#include <list>

namespace carto {

    template <typename T>
    class KDTreeSpatialIndex : public SpatialIndex<T> {
    public:
        KDTreeSpatialIndex();
        
        virtual std::size_t size() const;
        
        virtual void clear();
        virtual void insert(const MapBounds& bounds, const T& object);
        virtual bool remove(const MapBounds& bounds, const T& object);
        virtual bool remove(const T& object);
        
        virtual std::vector<T> query(const Frustum& frustum) const;
        virtual std::vector<T> query(const MapBounds& bounds) const;
        virtual std::vector<T> getAll() const;
        
    private:
        class Record {
        public:
            Record(const MapBounds& bounds, const T& object);
            
            MapBounds bounds;
            T object;
        };
        
        class Node {
        public:
            Node(const MapBounds& bounds);
            
            MapBounds bounds;
            std::list<Record> records;
            std::vector<std::shared_ptr<Node> > children; // <, >= half-planes
            int axis; // splitting axis (0 is x, 1 is y, 2 is z)
            double distance; // only defined if children != null, once defined, becomes immutable
        };
        
        static const int MAX_DEPTH = 20;
        static const std::size_t MIN_SPLIT_COUNT = 2;
        
        void insertToNode(const std::shared_ptr<Node>& node, const MapBounds& bounds, const T& object, int depth);
        std::shared_ptr<Node> removeFromNode(const std::shared_ptr<Node>& node, const MapBounds* bounds, const T& object);
        
        void queryNode(const std::shared_ptr<Node>& node, const Frustum& frustum, std::vector<T>& results) const;
        void queryNode(const std::shared_ptr<Node>& node, const MapBounds& bounds, std::vector<T>& results) const;
        void getAllFromNode(const std::shared_ptr<Node>& node, std::vector<T>& results) const;
        
        std::shared_ptr<Node> _root;
        std::size_t _count;
    };
    
    template<typename T>
    KDTreeSpatialIndex<T>::KDTreeSpatialIndex() :
        _root(),
        _count(0)
    {
    }
    
    template<typename T>
    std::size_t KDTreeSpatialIndex<T>::size() const {
        return _count;
    }
    
    template<typename T>
    void KDTreeSpatialIndex<T>::clear() {
        _root.reset();
        _count = 0;
    }
    
    template<typename T>
    void KDTreeSpatialIndex<T>::insert(const MapBounds& bounds, const T& object) {
        if (!_root) {
            _root = std::make_shared<Node>(bounds);
        }
        insertToNode(_root, bounds, object, 0);
    }
    
    template<typename T>
    bool KDTreeSpatialIndex<T>::remove(const MapBounds& bounds, const T& object) {
        std::size_t count = _count;
        _root = removeFromNode(_root, &bounds, object);
        return count != _count;
    }
    
    template<typename T>
    bool KDTreeSpatialIndex<T>::remove(const T& object) {
        std::size_t count = _count;
        _root = removeFromNode(_root, nullptr, object);
        return count != _count;
    }
    
    template<typename T>
    std::vector<T> KDTreeSpatialIndex<T>::query(const Frustum& frustum) const {
        std::vector<T> results;
        queryNode(_root, frustum, results);
        return results;
    }
    
    template<typename T>
    std::vector<T> KDTreeSpatialIndex<T>::query(const MapBounds& bounds) const {
        std::vector<T> results;
        queryNode(_root, bounds, results);
        return results;
    }
    
    template<typename T>
    std::vector<T> KDTreeSpatialIndex<T>::getAll() const {
        std::vector<T> results;
        getAllFromNode(_root, results);
        return results;
    }
    
    template<typename T>
    KDTreeSpatialIndex<T>::Record::Record(const MapBounds& bounds, const T& object) :
        bounds(bounds),
        object(object)
    {
    }
    
    template<typename T>
    KDTreeSpatialIndex<T>::Node::Node(const MapBounds& bounds) :
        bounds(bounds),
        records(),
        children(),
        axis(0),
        distance(-1)
    {
    }
    
    template<typename T>
    void KDTreeSpatialIndex<T>::insertToNode(const std::shared_ptr<Node>& node, const MapBounds& bounds, const T& object, int depth) {
        // Update node bounds
        node->bounds.expandToContain(bounds);
        
        // If depth limit has been exceeded, add to current node
        if (depth >= MAX_DEPTH) {
            node->records.emplace_back(bounds, object);
            _count++;
            return;
        }
        
        // Add to node records if this is leaf node
        if (node->children.empty()) {
            node->records.emplace_back(bounds, object);
            _count++;
            
            // Create children and redistribute node records among children if enough records have been added
            if (node->records.size() > MIN_SPLIT_COUNT) {
                int axis = 0;
                const MapVec& boundsDelta = node->bounds.getDelta();
                double biggestDelta = boundsDelta.getX();
                if (boundsDelta.getY() > biggestDelta) {
                    axis = 1;
                    biggestDelta = boundsDelta.getY();
                }
                if (boundsDelta.getZ() > biggestDelta) {
                    axis = 2;
                }
                
                std::vector<std::shared_ptr<Node> > children(2);
                double distance = node->bounds.getCenter()[axis];
                for (const Record& record : node->records) {
                    int index = record.bounds.getCenter()[axis] >= distance ? 1 : 0;
                    if (!children[index]) {
                        children[index] = std::make_shared<Node>(record.bounds);
                    } else {
                        children[index]->bounds.expandToContain(record.bounds);
                    }
                    children[index]->records.push_back(record);
                }
                
                // Check that split succeeded
                if (children[0] && children[1]) {
                    node->children = std::move(children);
                    node->axis = axis;
                    node->distance = distance;
                    node->records.clear();
                }
            }
            return;
        }
        
        // Recurse to children
        int index = (node->bounds.getCenter()[node->axis] >= node->distance ? 1 : 0);
        if (!node->children[index]) {
            node->children[index] = std::make_shared<Node>(bounds);
        }
        insertToNode(node->children[index], bounds, object, depth + 1);
    }
    
    template<typename T>
    std::shared_ptr<typename KDTreeSpatialIndex<T>::Node> KDTreeSpatialIndex<T>::removeFromNode(const std::shared_ptr<Node>& node, const MapBounds* bounds, const T& object) {
        // Check if we need to proceed
        if (!node) {
            return node;
        }
        if (bounds && !node->bounds.intersects(*bounds)) {
            return node;
        }
        
        // Remove object from current node
        for (auto it = node->records.begin(); it != node->records.end(); ++it) {
            const Record& record = *it;
            if (record.object == object) {
                it = node->records.erase(it);
                _count--;
            }
        }
        
        // Recurse and prune
        bool empty = true;
        for (std::size_t i = 0; i < node->children.size(); i++) {
            node->children[i] = removeFromNode(node->children[i], bounds, object);
            if (node->children[i]) {
                empty = false;
            }
        }
        if (empty) {
            node->children.clear();
        }
        return node->records.empty() && node->children.empty() ? std::shared_ptr<Node>() : node;
    }
    
    template<typename T>
    void KDTreeSpatialIndex<T>::queryNode(const std::shared_ptr<Node>& node, const Frustum& frustum, std::vector<T>& results) const {
        // Check if this node intersects with given envelope
        if (!node) {
            return;
        }
        if (!frustum.cuboidIntersects(node->bounds)) {
            return;
        }
        
        // Test for intersection of current node records
        for (typename std::list<Record>::iterator it = node->records.begin(); it != node->records.end(); ++it) {
            const Record& record = *it;
            if (frustum.cuboidIntersects(record.bounds)) {
                results.push_back(record.object);
            }
        }
        
        // Recurse
        for (const std::shared_ptr<Node>& child : node->children) {
            queryNode(child, frustum, results);
        }
    }
    
    template<typename T>
    void KDTreeSpatialIndex<T>::queryNode(const std::shared_ptr<Node>& node, const MapBounds& bounds, std::vector<T>& results) const {
        // Check if this node intersects with given envelope
        if (!node) {
            return;
        }
        if (!bounds.intersects(node->bounds)) {
            return;
        }
        
        // Test for intersection of current node records
        for (typename std::list<Record>::iterator it = node->records.begin(); it != node->records.end(); ++it) {
            const Record& record = *it;
            if (bounds.intersects(record.bounds)) {
                results.push_back(record.object);
            }
        }
        
        // Recurse
        for (const std::shared_ptr<Node>& child : node->children) {
            queryNode(child, bounds, results);
        }
    }
    
    template<typename T>
    void KDTreeSpatialIndex<T>::getAllFromNode(const std::shared_ptr<Node>& node, std::vector<T>& results) const {
        if (!node) {
            return;
        }
        
        // All objects under this node
        for (const Record& record : node->records) {
            results.push_back(record.object);
        }
        
        // Recurse
        for (const std::shared_ptr<Node>& child : node->children) {
            getAllFromNode(child, results);
        }
    }
    
}

#endif
