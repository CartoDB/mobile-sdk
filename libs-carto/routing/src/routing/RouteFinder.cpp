#include "RouteFinder.h"

namespace carto { namespace routing {
    Result RouteFinder::find(const Query& query) const {
        std::array<std::vector<Graph::NearestNode>, 2> nearestNodes;
        std::array<std::priority_queue<SearchNode>, 2> heaps;
        std::unordered_map<Graph::NodeId, PathNode, Graph::NodeId::Hash> pathSuffixMap;
        float minWeight = 0.0f;
        for (int i = 0; i < 2; i++) {
            nearestNodes[i] = _graph->findNearestNode(query.getPos(i));
            if (nearestNodes[i].empty()) {
                return Result();
            }

            for (const Graph::NearestNode& nearestNode : nearestNodes[i]) {
                Graph::NodePtr node = _graph->getNode(nearestNode.nodeId);

                // Calculate end-point weights
                float weight = (i == 0 ? -nearestNode.geometryRelPos : nearestNode.geometryRelPos) * node->nodeData.weight;
                minWeight = std::min(minWeight, weight);

                // Special case: we have already added same node but the node is inaccessible along the current direction
                if (i == 1 && nearestNodes[0].size() == 1 && nearestNodes[1].size() == 1) {
                    const Graph::NearestNode& otherNearestNode = nearestNodes[1 - i][0];
                    if (nearestNode.nodeId == otherNearestNode.nodeId && nearestNode.geometryRelPos < otherNearestNode.geometryRelPos) {
                        // Add all backward edges "leading" to current node
                        for (auto edge = node->firstEdge; edge != node->lastEdge; edge++) {
                            if (edge->backward) {
                                heaps[i].emplace(edge->targetNodeId, Graph::NodeId(), weight + edge->edgeData.weight);
                                pathSuffixMap[edge->targetNodeId] = PathNode(edge->targetNodeId, *edge, nearestNode.nodeId);
                            }
                        }

                        // Here comes the tricky part: we must perform another spatial query to find INCOMING edges pointing to current edge
                        std::vector<WGSPos> geometry = _graph->getNodeGeometry(*node);
                        std::vector<Graph::NearestNode> nearestNodes2 = _graph->findNearestNode(geometry.front());
                        for (const Graph::NearestNode& nearestNode2 : nearestNodes2) {
                            Graph::NodePtr node2 = _graph->getNode(nearestNode2.nodeId);
                            for (auto edge2 = node2->firstEdge; edge2 != node2->lastEdge; edge2++) {
                                if (edge2->forward && edge2->targetNodeId == nearestNode.nodeId) {
                                    heaps[i].emplace(nearestNode2.nodeId, Graph::NodeId(), weight + edge2->edgeData.weight);
                                    pathSuffixMap[nearestNode2.nodeId] = PathNode(nearestNode2.nodeId, *edge2, nearestNode.nodeId);
                                }
                            }
                        }

                        continue;
                    }
                }

                // Add the node to heap, if other nodes were not already added
                heaps[i].emplace(nearestNode.nodeId, Graph::NodeId(), weight);
            }
        }

        // Apply bidirectional Dijkstra
        Graph::NodeId bestNodeId;
        float bestWeight = std::numeric_limits<float>::infinity();
        std::array<std::unordered_map<Graph::NodeId, SearchNode, Graph::NodeId::Hash>, 2> settledNodes;
        for (int i = 0; !(heaps[0].empty() && heaps[1].empty()); i = 1 - i) {
            if (heaps[i].empty()) {
                continue;
            }
            SearchNode searchNode = heaps[i].top();
            heaps[i].pop();
            
            // Skip all invalid nodes
            if (searchNode.nodeId.blockId.packageId == -1) {
                continue;
            }

            // Already shorter path found? In that case we can stop searching in the given direction
            if (searchNode.weight + minWeight > bestWeight) {
                while (!heaps[i].empty()) {
                    heaps[i].pop();
                }
                continue;
            }
            
            // Best path to give node?
            auto it0 = settledNodes[i].find(searchNode.nodeId);
            if (it0 != settledNodes[i].end()) {
                if (searchNode.weight >= it0->second.weight) { // not sure if the check is required
                    continue;
                }
            }

            // Settle the node
            settledNodes[i][searchNode.nodeId] = searchNode;
            
            // Stalling optimization. This implementation is not optimal, we should also look at non-settled heap nodes
            Graph::NodePtr node = _graph->getNode(searchNode.nodeId);
            bool stall = false;
            for (auto edge = node->firstEdge; edge != node->lastEdge; edge++) {
                if ((i == 0 && edge->backward) || (i != 0 && edge->forward)) {
                    auto it = settledNodes[i].find(edge->targetNodeId);
                    if (it != settledNodes[i].end()) {
                        if (it->second.weight + edge->edgeData.weight < searchNode.weight) {
                            stall = true;
                            break;
                        }
                    }
                }
            }
            if (stall) {
                continue;
            }

            // Recalculate shortest path and middle node
            auto it1 = settledNodes[1 - i].find(searchNode.nodeId);
            if (it1 != settledNodes[1 - i].end()) {
                float totalWeight = searchNode.weight + it1->second.weight;
                if (totalWeight >= 0 && totalWeight < bestWeight) {
                    bestWeight = totalWeight;
                    bestNodeId = searchNode.nodeId;
                }
            }

            // Add target nodes to heap
            for (auto edge = node->firstEdge; edge != node->lastEdge; edge++) {
                if ((i == 0 && edge->forward) || (i != 0 && edge->backward)) {
                    heaps[i].emplace(edge->targetNodeId, searchNode.nodeId, searchNode.weight + edge->edgeData.weight);
                }
            }
        }

        // Check that path was found
        if (bestNodeId.blockId.packageId == -1) {
            return Result();
        }

        // Unpack path
        std::array<std::vector<PathNode>, 2> paths;
        for (int i = 0; i < 2; i++) {
            std::stack<std::pair<Graph::NodeId, Graph::NodeId>> stack;
            Graph::NodeId nodeId = bestNodeId;
            while (true) {
                auto it = settledNodes[i].find(nodeId);
                assert(it != settledNodes[i].end());
                Graph::NodeId prevNodeId = it->second.prevNodeId;
                if (prevNodeId.blockId.packageId == -1) {
                    break;
                }
                stack.emplace(prevNodeId, nodeId);
                nodeId = prevNodeId;
            }

            while (!stack.empty()) {
                std::pair<Graph::NodeId, Graph::NodeId> nodeIds = stack.top();
                stack.pop();

                Graph::NodePtr matchedNode;
                const Graph::Edge* matchedEdge = nullptr;

                // Find the edge between prevNodeId and nodeId. Do matching based on node ids.
                Graph::NodeId prevNodeId = nodeIds.first;
                Graph::NodeId nodeId = nodeIds.second;
                for (int j = 0; j < 2 && !matchedEdge; j++) {
                    Graph::NodePtr prevNode = _graph->getNode(prevNodeId);
                    for (auto edge = prevNode->firstEdge; edge != prevNode->lastEdge; edge++) {
                        if (edge->targetNodeId == nodeId && ((j == i && edge->forward) || (j != i && edge->backward))) {
                            matchedNode = prevNode;
                            matchedEdge = edge;
                            break;
                        }
                    }
                    std::swap(nodeId, prevNodeId);
                }
                
                // If the edge was not found, then we have a link between packages with different node encodings. Do slow matching, based on geometry, not node ids
                for (int j = 0; j < 2 && !matchedEdge; j++) {
                    Graph::NodePtr prevNode = _graph->getNode(prevNodeId);
                    Graph::NodePtr node = _graph->getNode(nodeId);
                    std::vector<WGSPos> nodeGeometry = _graph->getNodeGeometry(*node);
                    for (auto edge = prevNode->firstEdge; edge != prevNode->lastEdge; edge++) {
                        if (edge->targetNodeId.blockId.packageId == -1) {
                            continue;
                        }
                        Graph::NodePtr targetNode = _graph->getNode(edge->targetNodeId);
                        std::vector<WGSPos> targetNodeGeometry = _graph->getNodeGeometry(*targetNode);
                        if (nodeGeometry == targetNodeGeometry && ((j == i && edge->forward) || (j != i && edge->backward))) {
                            matchedNode = prevNode;
                            matchedEdge = edge;
                            break;
                        }
                    }
                    std::swap(nodeId, prevNodeId);
                }
                
                // Unpack the matched edge
                if (matchedEdge) {
                    if (matchedEdge->contracted) {
                        if (matchedEdge->contractedNodeId.blockId.packageId == -1) {
                            return Result(); // Contracted node is not available, packing failed
                        }
                        stack.emplace(matchedEdge->contractedNodeId, nodeIds.second);
                        stack.emplace(nodeIds.first, matchedEdge->contractedNodeId);
                    }
                    else {
                        paths[i].emplace_back(nodeIds.first, *matchedEdge, nodeIds.second);
                    }
                } else {
                    return Result(); // NOTE: this should not happen, unless the graph is broken
                }
            }
        }

        // Build joined path. Add pseudo-node at the beginning to simplify processing and add final node, if rerouting in case of one-way street
        std::vector<PathNode> path;
        path = paths[0];
        for (auto it = paths[1].rbegin(); it != paths[1].rend(); it++) {
            path.emplace_back(it->nextNodeId, it->edge, it->prevNodeId);
        }
        if (path.empty()) {
            path.emplace(path.begin(), bestNodeId, Graph::Edge(), bestNodeId);
        }
        else {
            Graph::NodeId firstNodeId = path.front().prevNodeId;
            path.emplace(path.begin(), firstNodeId, Graph::Edge(), firstNodeId);
        }

        auto finalNodeIt = pathSuffixMap.find(path.back().nextNodeId);
        if (finalNodeIt != pathSuffixMap.end()) {
            path.push_back(finalNodeIt->second);
        }

        // Construct query result
        std::vector<Instruction> instructions;
        std::vector<WGSPos> routeVertices;
        for (std::size_t j = 0; j < path.size(); j++) {
            Graph::NodeId nodeId = path[j].nextNodeId;
            Graph::NodePtr node = _graph->getNode(nodeId);
            
            std::vector<WGSPos> geometry = _graph->getNodeGeometry(*node);
            std::pair<std::size_t, std::size_t> geometryIndex(0, geometry.size());
            std::pair<float, float> geometryRelPos(0.0f, 1.0f);

            std::size_t firstNNIndex = std::numeric_limits<std::size_t>::max();
            if (j == 0) {
                for (std::size_t k = 0; k < nearestNodes[0].size(); k++) {
                    if (nearestNodes[0][k].nodeId == nodeId) {
                        geometryIndex.first = nearestNodes[0][k].geometrySegmentIndex;
                        geometryRelPos.first = nearestNodes[0][k].geometryRelPos;
                        firstNNIndex = k;
                        break;
                    }
                }
            }

            std::size_t lastNNIndex = std::numeric_limits<std::size_t>::max();
            if (j == path.size() - 1) {
                for (std::size_t k = 0; k < nearestNodes[1].size(); k++) {
                    if (nearestNodes[1][k].nodeId == nodeId) {
                        geometryIndex.second = nearestNodes[1][k].geometrySegmentIndex;
                        geometryRelPos.second = nearestNodes[1][k].geometryRelPos;
                        lastNNIndex = k;
                        break;
                    }
                }
            }

            double dist = calculateGeometryLength(geometry, geometryRelPos.first, geometryRelPos.second);
            double time = (j > 0 ? path[j].edge.edgeData.weight : node->nodeData.weight) * (geometryRelPos.second - geometryRelPos.first) / 10.0;
            std::string streetName = _graph->getNodeName(*node);

            // Initial route instruction/vertex
            if (firstNNIndex != std::numeric_limits<std::size_t>::max()) {
                instructions.emplace_back(Instruction::Type::HEAD_ON, Instruction::TravelMode::DEFAULT, streetName, dist, time, routeVertices.size());
                routeVertices.push_back(nearestNodes[0][firstNNIndex].nodePos);
            }
            
            // Middle instructions/vertices
            if (!routeVertices.empty() && geometryIndex.first < geometryIndex.second) {
                if (routeVertices.back() == geometry[geometryIndex.first]) {
                    routeVertices.pop_back();
                }
            }
            std::size_t vertexIndex = routeVertices.size();
            routeVertices.insert(routeVertices.end(), geometry.begin() + geometryIndex.first, geometry.begin() + geometryIndex.second);
            if (j > 0) {
                Instruction::Type type = static_cast<Instruction::Type>(path[j].edge.edgeData.turnInstruction);
                Instruction::TravelMode travelMode = static_cast<Instruction::TravelMode>(node->nodeData.travelMode);
                instructions.emplace_back(type, travelMode, streetName, dist, time, vertexIndex);
            }

            // Final instruction/vertex
            if (lastNNIndex != std::numeric_limits<std::size_t>::max()) {
                instructions.emplace_back(Instruction::Type::REACHED_YOUR_DESTINATION, Instruction::TravelMode::DEFAULT, "", 0, 0, routeVertices.size());
                routeVertices.push_back(nearestNodes[1][lastNNIndex].nodePos);
            }
        }

        return Result(std::move(instructions), std::move(routeVertices));
    }

    double RouteFinder::calculateGeometryLength(const std::vector<WGSPos>& geometry, double t0, double t1) {
        double totalLen = 0;
        for (unsigned int j = 1; j < geometry.size(); j++) {
            totalLen += calculateGreatCircleDistance(geometry[j - 1], geometry[j]);
        }
        if (t0 == 0 && t1 == 1) {
            return totalLen;
        }

        double pos = 0;
        double len = 0;
        for (unsigned int j = 1; j < geometry.size(); j++) {
            double segmentLen = calculateGreatCircleDistance(geometry[j - 1], geometry[j]);
            double segmentPos0 = std::max(pos, t0 * totalLen);
            double segmentPos1 = std::min(pos + segmentLen, t1 * totalLen);
            len += std::max(0.0, segmentPos1 - segmentPos0);
            pos += segmentLen;
        }
        return len;
    }

    double RouteFinder::calculateGreatCircleDistance(const WGSPos& p0, const WGSPos& p1) {
        double lat1 = p0(0) * DEG_TO_RAD;
        double lng1 = p0(1) * DEG_TO_RAD;
        double lat2 = p1(0) * DEG_TO_RAD;
        double lng2 = p1(1) * DEG_TO_RAD;

        double dLng = lng1 - lng2;
        double dLat = lat1 - lat2;

        double aHarv = std::pow(std::sin(dLat / 2.0), 2) + std::cos(lat1) * std::cos(lat2) * std::pow(std::sin(dLng / 2.0), 2);
        double cHarv = 2.0 * std::atan2(std::sqrt(aHarv), std::sqrt(1.0 - aHarv));
        return EARTH_RADIUS * cHarv;
    }
} }
