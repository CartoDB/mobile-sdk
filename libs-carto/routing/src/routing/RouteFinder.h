/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTING_ROUTEFINDER_H_
#define _CARTO_ROUTING_ROUTEFINDER_H_

#include "Query.h"
#include "Instruction.h"
#include "Result.h"
#include "Graph.h"

#include <queue>
#include <map>
#include <vector>
#include <stack>

namespace carto { namespace routing {
    class RouteFinder {
    public:
        explicit RouteFinder(std::shared_ptr<Graph> graph) : _graph(std::move(graph)) { }

        Result find(const Query& query) const;

    private:
        constexpr static double EARTH_RADIUS = 6372797.560856;

        struct SearchNode {
            Graph::NodeId nodeId;
            Graph::NodeId prevNodeId;
            float weight = 0.0f;

            SearchNode() = default;
            SearchNode(Graph::NodeId nodeId, Graph::NodeId prevNodeId, float weight) : nodeId(nodeId), prevNodeId(prevNodeId), weight(weight) { }

            bool operator < (const SearchNode& searchNode) const {
                return weight > searchNode.weight;
            }
        };

        struct PathNode {
            Graph::NodeId prevNodeId;
            Graph::Edge edge;
            Graph::NodeId nextNodeId;

            PathNode() = default;
            PathNode(Graph::NodeId prevNodeId, const Graph::Edge& edge, Graph::NodeId nextNodeId) : prevNodeId(prevNodeId), edge(edge), nextNodeId(nextNodeId) { }
        };

        static double calculateGeometryLength(const std::vector<WGSPos>& geometry, double t0, double t1);

        static double calculateGreatCircleDistance(const WGSPos& p0, const WGSPos& p1);

        const std::shared_ptr<Graph> _graph;
    };
} }

#endif
