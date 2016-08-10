/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTING_ROUTEFINDER_H_
#define _CARTO_ROUTING_ROUTEFINDER_H_

#include "RoutingObjects.h"
#include "RoutingGraph.h"

#include <queue>
#include <map>
#include <vector>
#include <stack>

namespace carto { namespace routing {
    class RouteFinder {
    public:
        explicit RouteFinder(std::shared_ptr<RoutingGraph> graph) : _graph(std::move(graph)) { }

        RoutingResult find(const RoutingQuery& query) const;

    private:
        struct SearchNode {
            RoutingGraph::NodeId nodeId;
            RoutingGraph::NodeId prevNodeId;
            float weight = 0.0f;

            SearchNode() = default;
            SearchNode(RoutingGraph::NodeId nodeId, RoutingGraph::NodeId prevNodeId, float weight) : nodeId(nodeId), prevNodeId(prevNodeId), weight(weight) { }

            bool operator < (const SearchNode& searchNode) const {
                return weight > searchNode.weight;
            }
        };

        struct PathNode {
            RoutingGraph::NodeId prevNodeId;
            RoutingGraph::Edge edge;
            RoutingGraph::NodeId nextNodeId;

            PathNode() = default;
            PathNode(RoutingGraph::NodeId prevNodeId, const RoutingGraph::Edge& edge, RoutingGraph::NodeId nextNodeId) : prevNodeId(prevNodeId), edge(edge), nextNodeId(nextNodeId) { }
        };

        static double calculateGeometryLength(const std::vector<WGSPos>& geometry, double t0, double t1);

        static double calculateGreatCircleDistance(const WGSPos& p0, const WGSPos& p1);

        const std::shared_ptr<RoutingGraph> _graph;
    };
} }

#endif
