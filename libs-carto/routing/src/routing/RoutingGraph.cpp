#include "RoutingGraph.h"

#include <cstdint>
#include <cstddef>
#include <list>
#include <queue>
#include <unordered_set>

#include <utf8.h>

namespace carto { namespace Routing {
    RoutingGraph::RoutingGraph(const Settings& settings) :
        _packages(),
        _nodeBlockCache(settings.nodeBlockCacheSize),
        _geometryBlockCache(settings.geometryBlockCacheSize),
        _nameBlockCache(settings.nameBlockCacheSize),
        _globalNodeBlockCache(settings.globalNodeBlockCacheSize),
        _rtreeNodeBlockCache(settings.rtreeNodeBlockCacheSize),
        _mutex()
    {
    }
    
    bool RoutingGraph::import(const std::string& fileName) {
        auto file = std::make_shared<std::ifstream>();
        file->exceptions(std::ifstream::failbit | std::ifstream::badbit);
#ifdef _WIN32
        std::wstring wfileName;
        utf8::utf8to16(fileName.begin(), fileName.end(), std::back_inserter(wfileName));
        file->open(wfileName, std::ios::binary);
#else
        file->open(fileName, std::ios::binary);
#endif
        return import(file);
    }

    bool RoutingGraph::import(const std::shared_ptr<std::ifstream>& file) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        Package package;
        package.packageId = static_cast<int>(_packages.size());
        
        auto graphChunk = std::dynamic_pointer_cast<eiff::form_chunk>(eiff::read_chunk(file, true));
        if (!graphChunk) {
            throw std::runtime_error("Illegal graph file");
        }

        auto headerChunk = graphChunk->get<eiff::data_chunk>(eiff::chunk::tag_type {{ 'H', 'E', 'A', 'D' }});
        if (!headerChunk) {
            throw std::runtime_error("Graph missing header chunk");
        }

        std::vector<unsigned char> headerData;
        headerChunk->read(headerData);

        bitstreams::input_bitstream bs(std::move(headerData));
        auto version = bs.read_bits<int>(32);
        if (version != VERSION) {
            throw std::runtime_error("Unsupported graph version");
        }
        auto packageNameLength = bs.read_bits<int>(16);
        while (packageNameLength-- > 0) {
            package.packageName.append(1, bs.read_bits<char>(8));
        }
        auto lat0 = bs.read_bits<int>(32);
        auto lon0 = bs.read_bits<int>(32);
        auto lat1 = bs.read_bits<int>(32);
        auto lon1 = bs.read_bits<int>(32);
        package.bbox.min = fromPoint(Point(lat0, lon0));
        package.bbox.max = fromPoint(Point(lat1, lon1));
        
        package.nodeChunk = graphChunk->get<eiff::data_chunk>(eiff::chunk::tag_type {{ 'N', 'O', 'D', 'E' }});
        package.geometryChunk = graphChunk->get<eiff::data_chunk>(eiff::chunk::tag_type {{ 'G', 'E', 'O', 'M' }});
        package.nameChunk = graphChunk->get<eiff::data_chunk>(eiff::chunk::tag_type {{ 'N', 'A', 'M', 'E' }});
        package.globalNodeChunk = graphChunk->get<eiff::data_chunk>(eiff::chunk::tag_type {{ 'L', 'I', 'N', 'K' }});
        package.rtreeNodeChunk = graphChunk->get<eiff::data_chunk>(eiff::chunk::tag_type {{ 'R', 'T', 'R', 'E' }});
        if (!package.nodeChunk || !package.geometryChunk || !package.nameChunk || !package.globalNodeChunk || !package.rtreeNodeChunk) {
            throw std::runtime_error("Graph sections missing");
        }
        _packages.push_back(std::move(package));

        // Invalidate caches whose contents may depend on other packages
        _nodeBlockCache.clear();
        _globalNodeBlockCache.clear();
        return true;
    }

    RoutingGraph::NodePtr RoutingGraph::getNode(NodeId nodeId) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        std::shared_ptr<NodeBlock> nodeBlock;
        if (!_nodeBlockCache.read(nodeId.blockId, nodeBlock)) {
            nodeBlock = loadNodeBlock(nodeId.blockId);
            _nodeBlockCache.put(nodeId.blockId, nodeBlock);
        }
        return NodePtr(nodeBlock, nodeId.elementIndex);
    }

    std::string RoutingGraph::getNodeName(const Node& node) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        NameId nameId = node.nodeData.nameId;
        std::shared_ptr<NameBlock> nameBlock;
        if (!_nameBlockCache.read(nameId.blockId, nameBlock)) {
            nameBlock = loadNameBlock(nameId.blockId);
            _nameBlockCache.put(nameId.blockId, nameBlock);
        }
        return nameBlock->names.at(nameId.elementIndex);
    }

    std::vector<WGSPos> RoutingGraph::getNodeGeometry(const Node& node) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        GeometryId geometryId = node.nodeData.geometryId;
        std::shared_ptr<GeometryBlock> geometryBlock;
        if (!_geometryBlockCache.read(geometryId.blockId, geometryBlock)) {
            geometryBlock = loadGeometryBlock(geometryId.blockId);
            _geometryBlockCache.put(geometryId.blockId, geometryBlock);
        }

        std::vector<WGSPos> geometry;
        geometry.reserve(geometryBlock->geometries.at(geometryId.elementIndex).size());
        for (const Point& point : geometryBlock->geometries.at(geometryId.elementIndex)) {
            geometry.emplace_back(fromPoint(point));
        }
        if (node.nodeData.geometryReversed) {
            std::reverse(geometry.begin(), geometry.end());
        }
        return geometry;
    }

    std::vector<RoutingGraph::NearestNode> RoutingGraph::findNearestNode(const WGSPos& pos) const {
        static const double DIST_THRESHOLD = 1.01;
        
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // First build a priority queue of the packages, based on distance from package bounding box
        std::priority_queue<SearchRTreeNode> searchRTreeNodeQueue;
        for (const Package& package : _packages) {
            double dist = getBBoxDistance(pos, package.bbox);
            searchRTreeNodeQueue.emplace(RTreeNodeId(BlockId(package.packageId, 0), 0), dist);
        }

        // Process the queue in order, with early out
        double bestDist = std::numeric_limits<double>::infinity();
        std::vector<NearestNode> bestNodes;
        while (!searchRTreeNodeQueue.empty()) {
            SearchRTreeNode searchRTreeNode = searchRTreeNodeQueue.top();
            if (searchRTreeNode.distance > bestDist * DIST_THRESHOLD) {
                break;
            }
            searchRTreeNodeQueue.pop();

            // Add all children of the node to the queue
            RTreeNode rtreeNode = loadRTreeNode(searchRTreeNode.rtreeNodeId);
            for (const std::pair<WGSBounds, RTreeNodeId>& child : rtreeNode.children) {
                double dist = getBBoxDistance(pos, child.first);
                searchRTreeNodeQueue.emplace(child.second, dist);
            }
            for (const std::pair<WGSBounds, BlockId>& nodeBlockId : rtreeNode.nodeBlockIds) {
                double dist = getBBoxDistance(pos, nodeBlockId.first);
                if (dist > bestDist * DIST_THRESHOLD) {
                    continue;
                }

                BlockId blockId = nodeBlockId.second;
                std::shared_ptr<NodeBlock> nodeBlock;
                if (!_nodeBlockCache.read(blockId, nodeBlock)) {
                    nodeBlock = loadNodeBlock(blockId);
                    _nodeBlockCache.put(blockId, nodeBlock);
                }

                // Fill bounds cache for the node block, if not yet created
                if (nodeBlock->nodeGeometryBoundsCache.empty()) {
                    nodeBlock->nodeGeometryBoundsCache.reserve(nodeBlock->nodes.size());
                    for (unsigned int i = 0; i < nodeBlock->nodes.size(); i++) {
                        const Node& node = nodeBlock->nodes[i];
                        std::vector<WGSPos> geometry = getNodeGeometry(node);
                        nodeBlock->nodeGeometryBoundsCache.push_back(WGSBounds::make_union(geometry.begin(), geometry.end()));
                    }
                }

                // Build priority queue of the nodes within the block, using distance to geometry bounding box
                std::priority_queue<SearchGeometry> searchGeometryQueue;
                for (unsigned int i = 0; i < nodeBlock->nodeGeometryBoundsCache.size(); i++) {
                    double dist = getBBoxDistance(pos, nodeBlock->nodeGeometryBoundsCache[i]);
                    if (dist <= bestDist * DIST_THRESHOLD) {
                        searchGeometryQueue.emplace(NodeId(blockId, i), dist);
                    }
                }

                // Process the node priority queue, with early out
                while (!searchGeometryQueue.empty()) {
                    SearchGeometry searchGeometry = searchGeometryQueue.top();
                    if (searchGeometry.distance > bestDist * DIST_THRESHOLD) {
                        break;
                    }
                    searchGeometryQueue.pop();

                    std::vector<WGSPos> geometry = getNodeGeometry(nodeBlock->nodes[searchGeometry.nodeId.elementIndex]);
                    double t = 0;
                    for (unsigned int j = 1; j < geometry.size(); j++) {
                        WGSPos posProj = getClosestSegmentPoint(pos, geometry[j - 1], geometry[j]);
                        double dist = getPointDistance(pos, posProj);
                        if (dist <= bestDist * DIST_THRESHOLD) {
                            if (dist * DIST_THRESHOLD < bestDist) {
                                bestNodes.clear();
                            }
                            bestDist = std::min(dist, bestDist);
                            
                            double len = 0;
                            for (unsigned int j = 1; j < geometry.size(); j++) {
                                len += cglib::length(geometry[j] - geometry[j - 1]);
                            }
                            
                            NearestNode newBestNode;
                            newBestNode.nodePos = posProj;
                            newBestNode.nodeId = searchGeometry.nodeId;
                            newBestNode.geometrySegmentIndex = j;
                            newBestNode.geometryRelPos = static_cast<float>((t + cglib::length(posProj - geometry[j - 1])) / len);
                            bestNodes.push_back(newBestNode);
                        }
                        t += cglib::length(geometry[j] - geometry[j - 1]);
                    }
                }
            }
        }
        return bestNodes;
    }
    
    std::shared_ptr<RoutingGraph::NodeBlock> RoutingGraph::loadNodeBlock(BlockId blockId) const {
        if (blockId.packageId == -1) {
            throw std::runtime_error("Bad package id");
        }

        const Package& package = _packages.at(blockId.packageId);

        std::vector<unsigned char> blockOffsetData(2 * sizeof(std::uint64_t));
        package.nodeChunk->read(blockOffsetData, sizeof(std::uint32_t) + blockId.blockIndex * sizeof(std::uint64_t), blockOffsetData.size());
        const std::uint64_t* blockOffsets = reinterpret_cast<std::uint64_t*>(blockOffsetData.data());

        std::vector<unsigned char> block;
        package.nodeChunk->read(block, blockOffsets[0], blockOffsets[1] - blockOffsets[0]);

        bitstreams::input_bitstream bs(std::move(block));

        auto nodeBlock = std::make_shared<NodeBlock>();

        // Read block header
        auto maxInternalNodeIndexBits = bs.read_bits<int>(6);
        auto maxExternalNodeBlockBits = bs.read_bits<int>(6);
        auto maxExternalNodeIndexBits = bs.read_bits<int>(6);
        auto maxGlobalNodeBlockBits = bs.read_bits<int>(6);
        auto maxGlobalNodeIndexBits = bs.read_bits<int>(6);
        auto maxContractedNodeBlockBits = bs.read_bits<int>(6);
        auto maxContractedNodeIndexBits = bs.read_bits<int>(6);
        auto maxGeometryBlockBits = bs.read_bits<int>(6);
        auto maxGeometryBlockDiffBits = bs.read_bits<int>(6);
        auto maxGeometryIndexBits = bs.read_bits<int>(6);
        auto maxNameBlockBits = bs.read_bits<int>(6);
        auto maxNameBlockDiffBits = bs.read_bits<int>(6);
        auto maxNameIndexBits = bs.read_bits<int>(6);
        auto maxNodeOutDegreeBits = bs.read_bits<int>(6);
        auto maxTravelModeBits = bs.read_bits<int>(6);
        auto maxInstructionBits = bs.read_bits<int>(6);
        auto smallWeightBits = bs.read_bits<int>(6);
        auto largeWeightBits = bs.read_bits<int>(6);
        auto minGeometryBlockId = bs.read_bits<unsigned int>(maxGeometryBlockBits);
        auto minNameBlockId = bs.read_bits<unsigned int>(maxNameBlockBits);

        // Store nodes and outgoing edges
        auto nodeCount = bs.read_bits<int>(32);
        nodeBlock->nodes.reserve(nodeCount);
        std::vector<unsigned> nodeEdgeCount;
        nodeEdgeCount.reserve(nodeCount);
        for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
            nodeBlock->nodes.emplace_back();
            Node& node = nodeBlock->nodes.back();
            auto edgeCount = bs.read_bits<int>(maxNodeOutDegreeBits);
            auto geometryBlockId = minGeometryBlockId + bs.read_bits<unsigned int>(maxGeometryBlockDiffBits);
            auto geometryIndexId = bs.read_bits<unsigned int>(maxGeometryIndexBits);
            node.nodeData.geometryId = GeometryId(BlockId(package.packageId, geometryBlockId), geometryIndexId);
            node.nodeData.geometryReversed = bs.read_bit();
            auto nameBlockId = minNameBlockId + bs.read_bits<unsigned int>(maxNameBlockDiffBits);
            auto nameIndexId = bs.read_bits<unsigned int>(maxNameIndexBits);
            node.nodeData.nameId = NameId(BlockId(package.packageId, nameBlockId), nameIndexId);
            node.nodeData.travelMode = bs.read_bits<unsigned char>(maxTravelModeBits);
            if (bs.read_bit()) {
                node.nodeData.weight = bs.read_bits<unsigned int>(largeWeightBits);
            }
            else {
                node.nodeData.weight = bs.read_bits<unsigned int>(smallWeightBits);
            }

            nodeEdgeCount.push_back(edgeCount);
            while (edgeCount-- > 0) {
                nodeBlock->edges.emplace_back();
                Edge& edge = nodeBlock->edges.back();
                if (bs.read_bit()) {
                    auto delta = bs.read_bits<unsigned int>(maxExternalNodeBlockBits);
                    auto targetBlockIndex = blockId.blockIndex - delta;
                    auto targetNodeIndex = bs.read_bits<unsigned int>(maxExternalNodeIndexBits);
                    edge.targetNodeId = NodeId(BlockId(package.packageId, targetBlockIndex), targetNodeIndex);
                }
                else {
                    auto delta = bs.read_bits<unsigned int>(maxInternalNodeIndexBits);
                    if (delta == 0) {
                        auto globalTargetBlockIndex = bs.read_bits<unsigned int>(maxGlobalNodeBlockBits);
                        auto globalTargetNodeIndex = bs.read_bits<unsigned int>(maxGlobalNodeIndexBits);
                        edge.targetNodeId = resolveGlobalNodeId(NodeId(BlockId(package.packageId, globalTargetBlockIndex), globalTargetNodeIndex));
                    }
                    else {
                        auto targetNodeIndex = nodeIndex - delta;
                        edge.targetNodeId = NodeId(blockId, targetNodeIndex);
                    }
                }
                edge.forward = bs.read_bit();
                edge.backward = bs.read_bit();
                if (bs.read_bit()) {
                    edge.edgeData.weight = bs.read_bits<unsigned int>(largeWeightBits);
                }
                else {
                    edge.edgeData.weight = bs.read_bits<unsigned int>(smallWeightBits);
                }
                if (bs.read_bit()) {
                    edge.contracted = true;
                    if (bs.read_bit()) {
                        auto delta = decodeZigZagValue(bs.read_bits<unsigned int>(maxContractedNodeBlockBits));
                        auto contractedBlockIndex = blockId.blockIndex + delta;
                        auto contractedNodeIndex = bs.read_bits<unsigned int>(maxContractedNodeIndexBits);
                        edge.contractedNodeId = NodeId(BlockId(package.packageId, contractedBlockIndex), contractedNodeIndex);
                    }
                    else {
                        auto delta = bs.read_bits<unsigned int>(maxInternalNodeIndexBits);
                        if (delta == 0) {
                            auto globalContractedBlockIndex = bs.read_bits<unsigned int>(maxGlobalNodeBlockBits);
                            auto globalContractedNodeIndex = bs.read_bits<unsigned int>(maxGlobalNodeIndexBits);
                            edge.contractedNodeId = resolveGlobalNodeId(NodeId(BlockId(package.packageId, globalContractedBlockIndex), globalContractedNodeIndex));
                        }
                        else {
                            auto contractedNodeIndex = nodeIndex - delta;
                            edge.contractedNodeId = NodeId(blockId, contractedNodeIndex);
                        }
                    }
                }
                else {
                    edge.contracted = false;
                    edge.edgeData.turnInstruction = bs.read_bits<unsigned char>(maxInstructionBits);
                }
            }
        }
        
        // Link node edge iterators to edge table
        unsigned edgeIndex = 0;
        for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
            Node& node = nodeBlock->nodes[nodeIndex];
            node.firstEdge = nodeBlock->edges.data() + edgeIndex;
            edgeIndex += nodeEdgeCount[nodeIndex];
            node.lastEdge = nodeBlock->edges.data() + edgeIndex;
        }
        if (edgeIndex != nodeBlock->edges.size()) {
            throw std::runtime_error("Block node/edge table is corrupted");
        }
        
        return nodeBlock;
    }

    std::shared_ptr<RoutingGraph::GeometryBlock> RoutingGraph::loadGeometryBlock(BlockId blockId) const {
        if (blockId.packageId == -1) {
            throw std::runtime_error("Bad package id");
        }

        const Package& package = _packages.at(blockId.packageId);

        std::vector<unsigned char> blockOffsetData(2 * sizeof(std::uint64_t));
        package.geometryChunk->read(blockOffsetData, sizeof(std::uint32_t) + blockId.blockIndex * sizeof(std::uint64_t), blockOffsetData.size());
        const std::uint64_t* blockOffsets = reinterpret_cast<std::uint64_t*>(blockOffsetData.data());

        std::vector<unsigned char> block;
        package.geometryChunk->read(block, blockOffsets[0], blockOffsets[1] - blockOffsets[0]);

        bitstreams::input_bitstream bs(std::move(block));

        auto geometryBlock = std::make_shared<GeometryBlock>();

        // Read block header
        auto maxLatDiffBits = bs.read_bits<int>(6);
        auto maxLonDiffBits = bs.read_bits<int>(6);
        auto maxGeometrySizeBits = bs.read_bits<int>(6);
        auto minLat = bs.read_bits<int>(32);
        auto minLon = bs.read_bits<int>(32);
        
        // Read geometry list
        auto geometryCount = bs.read_bits<int>(32);
        geometryBlock->geometries.reserve(geometryCount);
        while (geometryCount-- > 0) {
            auto maxLatZigZagBits = bs.read_bits<int>(6);
            auto maxLonZigZagBits = bs.read_bits<int>(6);
            auto lat = minLat + bs.read_bits<int>(maxLatDiffBits);
            auto lon = minLon + bs.read_bits<int>(maxLonDiffBits);

            // Read geometry delta encoded vertices
            auto geometrySize = bs.read_bits<int>(maxGeometrySizeBits);
            std::vector<Point> geometry;
            geometry.reserve(geometrySize + 1);
            geometry.emplace_back(lat, lon);
            while (geometrySize-- > 0) {
                lat += decodeZigZagValue(bs.read_bits<int>(maxLatZigZagBits));
                lon += decodeZigZagValue(bs.read_bits<int>(maxLonZigZagBits));
                geometry.emplace_back(lat, lon);
            }
            geometryBlock->geometries.push_back(std::move(geometry));
        }

        return geometryBlock;
    }

    std::shared_ptr<RoutingGraph::NameBlock> RoutingGraph::loadNameBlock(BlockId blockId) const {
        if (blockId.packageId == -1) {
            throw std::runtime_error("Bad package id");
        }

        const Package& package = _packages.at(blockId.packageId);

        std::vector<unsigned char> blockOffsetData(2 * sizeof(std::uint64_t));
        package.nameChunk->read(blockOffsetData, sizeof(std::uint32_t) + blockId.blockIndex * sizeof(std::uint64_t), blockOffsetData.size());
        const std::uint64_t* blockOffsets = reinterpret_cast<std::uint64_t*>(blockOffsetData.data());

        std::vector<unsigned char> block;
        package.nameChunk->read(block, blockOffsets[0], blockOffsets[1] - blockOffsets[0]);

        bitstreams::input_bitstream bs(std::move(block));

        auto nameBlock = std::make_shared<NameBlock>();

        // Read block header
        auto maxLengthBits = bs.read_bits<int>(6);

        // Read name list
        auto nameCount = bs.read_bits<int>(32);
        nameBlock->names.reserve(nameCount);
        while (nameCount-- > 0) {
            auto length = bs.read_bits<int>(maxLengthBits);
            std::string name;
            name.reserve(length);
            while (length-- > 0) {
                name.append(1, bs.read_bits<unsigned char>(8));
            }
            nameBlock->names.push_back(std::move(name));
        }

        return nameBlock;
    }
    
    std::shared_ptr<RoutingGraph::GlobalNodeBlock> RoutingGraph::loadGlobalNodeBlock(BlockId blockId) const {
        if (blockId.packageId == -1) {
            throw std::runtime_error("Bad package id");
        }
        
        const Package& package = _packages.at(blockId.packageId);
        
        std::vector<unsigned char> blockOffsetData(2 * sizeof(std::uint64_t));
        package.globalNodeChunk->read(blockOffsetData, sizeof(std::uint32_t) + blockId.blockIndex * sizeof(std::uint64_t), blockOffsetData.size());
        const std::uint64_t* blockOffsets = reinterpret_cast<std::uint64_t*>(blockOffsetData.data());
        
        std::vector<unsigned char> block;
        package.globalNodeChunk->read(block, blockOffsets[0], blockOffsets[1] - blockOffsets[0]);
        
        bitstreams::input_bitstream bs(std::move(block));
        
        auto globalNodeBlock = std::make_shared<GlobalNodeBlock>();
        
        auto maxPackageNameBits = bs.read_bits<int>(6);
        auto maxPackagesPerNodeBits = bs.read_bits<int>(6);
        auto maxGlobalNodeBlockBits = bs.read_bits<int>(6);
        auto maxGlobalNodeIndexBits = bs.read_bits<int>(6);
        
        std::vector<int> packageIds;
        auto packagesCount = bs.read_bits<int>(32);
        packageIds.reserve(packagesCount);
        while (packagesCount-- > 0) {
            std::string packageName;
            auto packageLength = bs.read_bits<int>(maxPackageNameBits);
            packageName.reserve(packageLength);
            while (packageLength-- > 0) {
                packageName.append(1, bs.read_bits<char>(8));
            }
            int packageId = -1;
            for (const Package& package : _packages) {
                if (package.packageName == packageName) {
                    packageId = package.packageId;
                    break;
                }
            }
            packageIds.push_back(packageId);
        }
        
        auto globalNodeCount = bs.read_bits<int>(32);
        globalNodeBlock->globalNodeIds.reserve(globalNodeCount);
        while (globalNodeCount-- > 0) {
            NodeId globalNodeId;
            auto nodePackagesCount = bs.read_bits<int>(maxPackagesPerNodeBits);
            while (nodePackagesCount-- > 0) {
                auto packageIndex = bs.read_bits<int>(maxPackagesPerNodeBits);
                auto blockIndex = bs.read_bits<int>(maxGlobalNodeBlockBits);
                auto nodeIndex = bs.read_bits<int>(maxGlobalNodeIndexBits);
                if (packageIds.at(packageIndex) != -1) {
                    globalNodeId = NodeId(BlockId(packageIds.at(packageIndex), blockIndex), nodeIndex);
                }
            }
            globalNodeBlock->globalNodeIds.push_back(globalNodeId);
        }

        return globalNodeBlock;
    }
    
    std::shared_ptr<RoutingGraph::RTreeNodeBlock> RoutingGraph::loadRTreeNodeBlock(BlockId blockId) const {
        if (blockId.packageId == -1) {
            throw std::runtime_error("Bad package id");
        }
        
        const Package& package = _packages.at(blockId.packageId);

        std::vector<unsigned char> blockOffsetData(2 * sizeof(std::uint64_t));
        package.rtreeNodeChunk->read(blockOffsetData, sizeof(std::uint32_t) + blockId.blockIndex * sizeof(std::uint64_t), blockOffsetData.size());
        const std::uint64_t* blockOffsets = reinterpret_cast<std::uint64_t*>(blockOffsetData.data());
        
        std::vector<unsigned char> block;
        package.rtreeNodeChunk->read(block, blockOffsets[0], blockOffsets[1] - blockOffsets[0]);
        
        bitstreams::input_bitstream bs(std::move(block));
        
        auto rtreeNodeBlock = std::make_shared<RTreeNodeBlock>();

        auto maxRTreeBlockBits = bs.read_bits<int>(6);
        auto maxRTreeIndexBits = bs.read_bits<int>(6);
        auto maxNodeBlockBits = bs.read_bits<int>(6);
        auto maxSizeBits = bs.read_bits<int>(6);
        auto maxLatDiffBits = bs.read_bits<int>(6);
        auto maxLonDiffBits = bs.read_bits<int>(6);
        auto maxLatDiffBits2 = bs.read_bits<int>(6);
        auto maxLonDiffBits2 = bs.read_bits<int>(6);
        
        auto minLat = bs.read_bits<int>(32);
        auto minLon = bs.read_bits<int>(32);
        
        auto nodeCount = bs.read_bits<int>(32);
        rtreeNodeBlock->rtreeNodes.reserve(nodeCount);
        while (nodeCount-- > 0) {
            RTreeNode rtreeNode;
            bool leaf = bs.read_bit();
            auto childCount = bs.read_bits<int>(maxSizeBits);
            if (leaf) {
                rtreeNode.nodeBlockIds.reserve(childCount);
            }
            else {
                rtreeNode.children.reserve(childCount);
            }
            while (childCount-- > 0) {
                auto lat0 = minLat + bs.read_bits<int>(maxLatDiffBits);
                auto lon0 = minLon + bs.read_bits<int>(maxLonDiffBits);
                auto lat1 = lat0 + bs.read_bits<int>(maxLatDiffBits2);
                auto lon1 = lon0 + bs.read_bits<int>(maxLonDiffBits2);
                WGSBounds bbox(fromPoint(Point(lat0, lon0)), fromPoint(Point(lat1, lon1)));
                if (leaf) {
                    auto nodeBlockId = bs.read_bits<unsigned int>(maxNodeBlockBits);
                    rtreeNode.nodeBlockIds.emplace_back(bbox, BlockId(package.packageId, nodeBlockId));
                }
                else {
                    auto rtreeNodeBlockId = bs.read_bits<unsigned int>(maxRTreeBlockBits);
                    auto rtreeNodeIndexId = bs.read_bits<unsigned int>(maxRTreeIndexBits);
                    rtreeNode.children.emplace_back(bbox, RTreeNodeId(BlockId(package.packageId, rtreeNodeBlockId), rtreeNodeIndexId));
                }
            }
            rtreeNodeBlock->rtreeNodes.push_back(std::move(rtreeNode));
        }

        return rtreeNodeBlock;
    }
    
    RoutingGraph::NodeId RoutingGraph::resolveGlobalNodeId(GlobalNodeId globalNodeId) const {
        std::shared_ptr<GlobalNodeBlock> globalNodeBlock;
        if (!_globalNodeBlockCache.read(globalNodeId.blockId, globalNodeBlock)) {
            globalNodeBlock = loadGlobalNodeBlock(globalNodeId.blockId);
            _globalNodeBlockCache.put(globalNodeId.blockId, globalNodeBlock);
        }
        return globalNodeBlock->globalNodeIds.at(globalNodeId.elementIndex);
    }

    RoutingGraph::RTreeNode RoutingGraph::loadRTreeNode(RTreeNodeId rtreeNodeId) const {
        std::shared_ptr<RTreeNodeBlock> rtreeNodeBlock;
        if (!_rtreeNodeBlockCache.read(rtreeNodeId.blockId, rtreeNodeBlock)) {
            rtreeNodeBlock = loadRTreeNodeBlock(rtreeNodeId.blockId);
            _rtreeNodeBlockCache.put(rtreeNodeId.blockId, rtreeNodeBlock);
        }
        return rtreeNodeBlock->rtreeNodes.at(rtreeNodeId.elementIndex);
    }
    
    WGSPos RoutingGraph::getClosestSegmentPoint(const WGSPos& pos, const WGSPos& p0, const WGSPos& p1) {
        // TODO: questionable approximation, we should project all positions to EPSG3857 and the result back
        double lonFactor = std::cos((p0(0) + p1(0)) * 0.5 * DEG_TO_RAD);
        cglib::vec2<double> dp = p1 - p0;
        dp(1) *= lonFactor;
        double len2 = cglib::dot_product(dp, dp);
        if (len2 == 0) {
            return p0;
        }
        cglib::vec2<double> dpos = pos - p0;
        dpos(1) *= lonFactor;
        double t = cglib::dot_product(dp, dpos);
        WGSPos posProj = p0 + cglib::vec2<double>(dp(0), dp(1) / lonFactor) * std::max(0.0, std::min(1.0, t / len2));
        return posProj;
    }
    
    double RoutingGraph::getBBoxDistance(const WGSPos& pos, const WGSBounds& bbox) {
        // TODO: we do not handle -180/180 wrapping properly
        double lonFactor = std::cos(pos(0) * DEG_TO_RAD);
        double dist = std::max(bbox.min(0) - pos(0), pos(0) - bbox.max(0));
        dist = std::max(dist, lonFactor * std::max(bbox.min(1) - pos(1), pos(1) - bbox.max(1)));
        return dist;
    }

    double RoutingGraph::getPointDistance(const WGSPos& pos0, const WGSPos& pos1) {
        // TODO: we do not handle -180/180 wrapping properly
        double lonFactor = std::cos((pos0(0) + pos1(0)) * 0.5 * DEG_TO_RAD);
        cglib::vec2<double> dp = pos1 - pos0;
        dp(1) *= lonFactor;
        return cglib::length(dp);
    }

    int RoutingGraph::decodeZigZagValue(unsigned int val) {
        return ((val & 1) != 0 ? -static_cast<int>((val + 1) >> 1) : static_cast<int>(val >> 1));
    }

    WGSPos RoutingGraph::fromPoint(const Point& point) {
        return WGSPos(point.lat * COORDINATE_SCALE, point.lon * COORDINATE_SCALE);
    }

    RoutingGraph::Point RoutingGraph::toPoint(const WGSPos& pos) {
        return Point(static_cast<int>(pos(0) / COORDINATE_SCALE), static_cast<int>(pos(1) / COORDINATE_SCALE));
    }
    
    const int RoutingGraph::VERSION = 0;

    const double RoutingGraph::COORDINATE_SCALE = 1.0e-6;

    const double RoutingGraph::DEG_TO_RAD = 0.017453292519943295769236907684886;
} }
