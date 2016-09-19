/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTING_GRAPH_H_
#define _CARTO_ROUTING_GRAPH_H_

#include "Base.h"

#include <memory>
#include <mutex>
#include <array>
#include <vector>
#include <fstream>
#include <utility>
#include <functional>

#include <stdext/lru_cache.h>
#include <stdext/eiff_file.h>
#include <stdext/bitstream.h>

namespace carto { namespace routing {
    class Graph {
    public:
        struct BlockId {
            int packageId = -1;
            int blockIndex = -1;
            
            BlockId() = default;
            explicit BlockId(int packageId, int blockIndex) : packageId(packageId), blockIndex(blockIndex) { }
            
            bool operator == (const BlockId& blockId) const { return packageId == blockId.packageId && blockIndex == blockId.blockIndex; }
            
            struct Hash {
                std::size_t operator() (const Graph::BlockId& blockId) const { return blockId.packageId * 98317 ^ blockId.blockIndex; }
            };
        };
        
        struct ElementId {
            BlockId blockId;
            int elementIndex = -1;
            
            ElementId() = default;
            ElementId(BlockId blockId, int elementIndex) : blockId(blockId), elementIndex(elementIndex) { }

            bool operator == (const ElementId& elementId) const { return blockId == elementId.blockId && elementIndex == elementId.elementIndex; }

            struct Hash {
                std::size_t operator() (const Graph::ElementId& elementId) const { return BlockId::Hash()(elementId.blockId) * 769 ^ elementId.elementIndex; }
            };
        };
        
        using GeometryId = ElementId;
        using NameId = ElementId;
        using NodeId = ElementId;
        using GlobalNodeId = ElementId;
        using RTreeNodeId = ElementId;

        struct Point {
            int lat = 0;
            int lon = 0;

            Point() = default;
            explicit Point(int lat, int lon) : lat(lat), lon(lon) { }
        };

        struct EdgeData {
            unsigned int weight = 0;
            unsigned char turnInstruction = 0; 

            EdgeData() = default;
        };

        struct Edge {
            NodeId targetNodeId;
            NodeId contractedNodeId;
            bool contracted = false;
            bool forward = false;
            bool backward = false;
            EdgeData edgeData;

            Edge() = default;
        };

        struct NodeData {
            GeometryId geometryId;
            bool geometryReversed = false;
            NameId nameId;
            unsigned int weight = 0;
            unsigned char travelMode = 0;

            NodeData() = default;
        };

        struct Node {
            const Edge* firstEdge = nullptr;
            const Edge* lastEdge = nullptr;
            NodeData nodeData;

            Node() = default;
        };
        
        struct RTreeNode {
            std::vector<std::pair<WGSBounds, RTreeNodeId>> children;
            std::vector<std::pair<WGSBounds, BlockId>> nodeBlockIds;
            
            RTreeNode() = default;
        };
        
        struct GeometryBlock {
            std::vector<std::vector<Point>> geometries;

            GeometryBlock() = default;
        };

        struct NameBlock {
            std::vector<std::string> names;

            NameBlock() = default;
        };

        struct NodeBlock {
            std::vector<Node> nodes;
            std::vector<Edge> edges;
            std::vector<WGSBounds> nodeGeometryBoundsCache;

            NodeBlock() = default;
        };
        
        struct GlobalNodeBlock {
            std::vector<NodeId> globalNodeIds;
            
            GlobalNodeBlock() = default;
        };
        
        struct RTreeNodeBlock {
            std::vector<RTreeNode> rtreeNodes;
            
            RTreeNodeBlock() = default;
        };
        
        struct NodePtr {
            NodePtr() = default;
            explicit NodePtr(const std::shared_ptr<NodeBlock>& nodeBlock, int elementIndex) : _node(&nodeBlock->nodes.at(elementIndex)), _nodeBlock(nodeBlock) { }

            const Node* operator -> () const { return _node; }
            const Node& operator * () const { return *_node; }

        private:
            const Node* _node = nullptr;
            std::shared_ptr<NodeBlock> _nodeBlock; // keep the node pointer valid by holding reference to the node block
        };

        struct NearestNode {
            WGSPos nodePos;
            NodeId nodeId;
            unsigned int geometrySegmentIndex = 0;
            float geometryRelPos = 0.0f;

            NearestNode() = default;
        };

        struct Settings {
            std::size_t nodeBlockCacheSize = 512;
            std::size_t geometryBlockCacheSize = 512;
            std::size_t nameBlockCacheSize = 64;
            std::size_t globalNodeBlockCacheSize = 64;
            std::size_t rtreeNodeBlockCacheSize = 16;

            Settings() = default;
        };

        Graph() = delete;
        explicit Graph(const Settings& settings);
        
        bool import(const std::string& fileName);
        bool import(const std::shared_ptr<std::ifstream>& file);

        NodePtr getNode(NodeId nodeId) const;
        std::string getNodeName(const Node& node) const;
        std::vector<WGSPos> getNodeGeometry(const Node& node) const;
        std::vector<NearestNode> findNearestNode(const WGSPos& pos) const;

    private:
        struct Package {
            int packageId = -1;
            std::string packageName;
            WGSBounds bbox = WGSBounds::smallest();
            std::shared_ptr<eiff::data_chunk> nodeChunk;
            std::shared_ptr<eiff::data_chunk> geometryChunk;
            std::shared_ptr<eiff::data_chunk> nameChunk;
            std::shared_ptr<eiff::data_chunk> globalNodeChunk;
            std::shared_ptr<eiff::data_chunk> rtreeNodeChunk;
            
            Package() = default;
        };
        
        struct SearchRTreeNode {
            RTreeNodeId rtreeNodeId;
            double distance = 0;
            
            SearchRTreeNode() = default;
            explicit SearchRTreeNode(RTreeNodeId rtreeNodeId, double distance) : rtreeNodeId(rtreeNodeId), distance(distance) { }
            
            bool operator < (const SearchRTreeNode& searchRTreeNode) const {
                return distance > searchRTreeNode.distance;
            }
        };
        
        struct SearchGeometry {
            NodeId nodeId;
            double distance = 0;
            
            SearchGeometry() = default;
            explicit SearchGeometry(NodeId nodeId, double distance) : nodeId(nodeId), distance(distance) { }
            
            bool operator < (const SearchGeometry& searchGeometry) const {
                return distance > searchGeometry.distance;
            }
        };
        
        std::shared_ptr<NodeBlock> loadNodeBlock(BlockId blockId) const;

        std::shared_ptr<GeometryBlock> loadGeometryBlock(BlockId blockId) const;

        std::shared_ptr<NameBlock> loadNameBlock(BlockId blockId) const;
        
        std::shared_ptr<GlobalNodeBlock> loadGlobalNodeBlock(BlockId blockId) const;
        
        std::shared_ptr<RTreeNodeBlock> loadRTreeNodeBlock(BlockId blockId) const;
        
        NodeId resolveGlobalNodeId(GlobalNodeId globalNodeId) const;
        
        RTreeNode loadRTreeNode(RTreeNodeId rtreeNodeId) const;

        static WGSPos getClosestSegmentPoint(const WGSPos& pos, const WGSPos& p0, const WGSPos& p1);
        
        static double getPointDistance(const WGSPos& pos0, const WGSPos& pos1);

        static double getBBoxDistance(const WGSPos& pos, const WGSBounds& bbox);
        
        static int decodeZigZagValue(unsigned int val);

        static WGSPos fromPoint(const Point& point);
        static Point toPoint(const WGSPos& pos);

        constexpr static int VERSION = 0;

        constexpr static double COORDINATE_SCALE = 1.0e-6;

        constexpr static double DEG_TO_RAD = 3.141592653589793 / 180.0;

        std::vector<Package> _packages;

        mutable cache::lru_cache<BlockId, std::shared_ptr<NodeBlock>, BlockId::Hash> _nodeBlockCache;
        mutable cache::lru_cache<BlockId, std::shared_ptr<GeometryBlock>, BlockId::Hash> _geometryBlockCache;
        mutable cache::lru_cache<BlockId, std::shared_ptr<NameBlock>, BlockId::Hash> _nameBlockCache;
        mutable cache::lru_cache<BlockId, std::shared_ptr<GlobalNodeBlock>, BlockId::Hash> _globalNodeBlockCache;
        mutable cache::lru_cache<BlockId, std::shared_ptr<RTreeNodeBlock>, BlockId::Hash> _rtreeNodeBlockCache;
        mutable std::recursive_mutex _mutex;
    };
} }

#endif
