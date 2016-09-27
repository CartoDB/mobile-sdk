#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "ValhallaOfflineRoutingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "routing/RoutingProxy.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/AssetUtils.h"

#include <vector>
#include <functional>
#include <string>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <sstream>
#include <strstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <valhalla/midgard/logging.h>
#include <valhalla/midgard/constants.h>
#include <valhalla/midgard/pointll.h>
#include <valhalla/baldr/json.h>
#include <valhalla/baldr/geojson.h>
#include <valhalla/baldr/pathlocation.h>
#include <valhalla/baldr/directededge.h>
#include <valhalla/loki/search.h>
#include <valhalla/sif/autocost.h>
#include <valhalla/sif/costfactory.h>
#include <valhalla/sif/bicyclecost.h>
#include <valhalla/sif/pedestriancost.h>
#include <valhalla/meili/measurement.h>
#include <valhalla/meili/map_matching.h>
#include <valhalla/meili/map_matcher.h>
#include <valhalla/meili/map_matcher_factory.h>
#include <valhalla/meili/match_result.h>
#include <valhalla/thor/pathalgorithm.h>
#include <valhalla/thor/bidirectional_astar.h>
#include <valhalla/thor/astar.h>
#include <valhalla/thor/multimodal.h>
#include <valhalla/thor/trippathbuilder.h>
#include <valhalla/thor/isochrone.h>
#include <valhalla/odin/util.h>
#include <valhalla/odin/directionsbuilder.h>
#include <valhalla/proto/trippath.pb.h>
#include <valhalla/proto/tripdirections.pb.h>

namespace valhalla { namespace thor {
    enum ACTION_TYPE {
        ROUTE = 0, VIAROUTE = 1, LOCATE = 2, ONE_TO_MANY = 3, MANY_TO_ONE = 4, MANY_TO_MANY = 5,
        SOURCES_TO_TARGETS = 6, OPTIMIZED_ROUTE = 7, ISOCHRONE = 8, ATTRIBUTES = 9
    };

    class thor_worker_t {
    public:
        enum ACTION_TYPE {
            ROUTE = 0, VIAROUTE = 1, LOCATE = 2, ONE_TO_MANY = 3, MANY_TO_ONE = 4, MANY_TO_MANY = 5,
            SOURCES_TO_TARGETS = 6, OPTIMIZED_ROUTE = 7, ISOCHRONE = 8, ATTRIBUTES = 9
        };
        thor_worker_t(const std::string& path, const std::string& costing) : reader(make_reader_config(path)) {
            factory.Register("pedestrian", sif::CreatePedestrianCost);
            //!!! other
            boost::property_tree::ptree config_costing;
            cost = factory.Create(costing, config_costing);
            mode = cost->travelmode();
            mode_costing[static_cast<uint32_t>(mode)] = cost;
        }
        virtual ~thor_worker_t() {
        }

        std::list<valhalla::odin::TripPath> path_depart_at(const std::vector<valhalla::midgard::PointLL>& points, const std::string &costing, const boost::optional<int> &date_time_type);

    protected:

        static boost::property_tree::ptree make_reader_config(const std::string& path) {
            boost::property_tree::ptree config;
            config.put("max_cache_size", 16 * 1024 * 1024);
            config.put("tile_dir", path);
            return config;
        }

        void update_origin(baldr::PathLocation& origin, bool prior_is_node, const baldr::GraphId& through_edge);
        void get_path(PathAlgorithm* path_algorithm, baldr::PathLocation& origin, baldr::PathLocation& destination, std::vector<thor::PathInfo>& path_edges);
        thor::PathAlgorithm* get_path_algorithm(const std::string& routetype, const baldr::PathLocation& origin, const baldr::PathLocation& destination);

        valhalla::sif::TravelMode mode;
        boost::optional<std::string> jsonp;
        std::vector<baldr::Location> locations;
        std::vector<baldr::Location> sources;
        std::vector<baldr::Location> targets;
        std::vector<baldr::PathLocation> correlated;
        std::vector<baldr::PathLocation> correlated_s;
        std::vector<baldr::PathLocation> correlated_t;
        sif::CostFactory<sif::DynamicCost> factory;
        sif::CostFactory<sif::DynamicCost>::cost_ptr_t cost;
        valhalla::sif::cost_ptr_t mode_costing[4];    // TODO - max # of modes?
        valhalla::baldr::GraphReader reader;
        // Path algorithms (TODO - perhaps use a map?))
        AStarPathAlgorithm astar;
        BidirectionalAStar bidir_astar;
        MultiModalPathAlgorithm multi_modal_astar;
        Isochrone isochrone_gen;
        float long_request;
        boost::optional<int> date_time_type;
    };

    thor::PathAlgorithm* thor_worker_t::get_path_algorithm(const std::string& routetype,
        const baldr::PathLocation& origin, const baldr::PathLocation& destination) {
        if (routetype == "multimodal") {
            return &multi_modal_astar;
        }
        else if (routetype == "bus") {
            // TODO - can we use bidirectional A*?
            return &astar;
        }
        else if (routetype == "pedestrian") {
            return &bidir_astar;
        }
        else {
            // Use A* if any origin and destination edges are the same - otherwise
            // use bidirectional A*. Bidirectional A* does not handle trivial cases
            // with oneways.
            for (auto& edge1 : origin.edges) {
                for (auto& edge2 : destination.edges) {
                    if (edge1.id == edge2.id) {
                        return &astar;
                    }
                }
            }
            return &bidir_astar;
        }
    }

    void thor_worker_t::get_path(PathAlgorithm* path_algorithm,
        baldr::PathLocation& origin, baldr::PathLocation& destination,
        std::vector<thor::PathInfo>& path_edges) {
        midgard::logging::Log("#_passes::1", " [ANALYTICS] ");
        // Find the path.
        path_edges = path_algorithm->GetBestPath(origin, destination, reader,
            mode_costing, mode);
        // If path is not found try again with relaxed limits (if allowed)
        if (path_edges.size() == 0) {
            valhalla::sif::cost_ptr_t cost = mode_costing[static_cast<uint32_t>(mode)];
            if (cost->AllowMultiPass()) {
                // 2nd pass. Less aggressive hierarchy transitioning
                path_algorithm->Clear();
                bool using_astar = (path_algorithm == &astar);
                float relax_factor = using_astar ? 16.0f : 8.0f;
                float expansion_within_factor = using_astar ? 4.0f : 2.0f;
                cost->RelaxHierarchyLimits(relax_factor, expansion_within_factor);
                midgard::logging::Log("#_passes::2", " [ANALYTICS] ");
                path_edges = path_algorithm->GetBestPath(origin, destination,
                    reader, mode_costing, mode);

                // 3rd pass (only for A*)
                if (path_edges.size() == 0 && using_astar) {
                    path_algorithm->Clear();
                    cost->DisableHighwayTransitions();
                    midgard::logging::Log("#_passes::3", " [ANALYTICS] ");
                    path_edges = path_algorithm->GetBestPath(origin, destination,
                        reader, mode_costing, mode);
                }
            }
        }
    }

    void thor_worker_t::update_origin(baldr::PathLocation& origin, bool prior_is_node,
        const baldr::GraphId& through_edge) {
        if (prior_is_node) {
            // TODO - remove the opposing through edge from list of edges unless
            // all outbound edges are entering noth_thru regions.
            // For now allow all edges
        }
        else {
            // Check if the edge is entering a not_thru region - if so do not
            // exclude the opposing edge
            const baldr::DirectedEdge* de = reader.GetGraphTile(through_edge)->directededge(through_edge);
            if (de->not_thru()) {
                return;
            }

            // Check if the through edge is dist = 1 (through point is at a node)
            bool ends_at_node = false;;
            for (const auto& e : origin.edges) {
                if (e.id == through_edge) {
                    if (e.end_node()) {
                        ends_at_node = true;
                        break;
                    }
                }
            }

            // Special case if location is at the end of a through edge
            if (ends_at_node) {
                // Erase the through edge and its opposing edge (if in the list)
                // from the origin edges
                auto opp_edge = reader.GetOpposingEdgeId(through_edge);
                std::remove_if(origin.edges.begin(), origin.edges.end(),
                    [&through_edge, &opp_edge](const baldr::PathLocation::PathEdge& edge) {
                    return edge.id == through_edge || edge.id == opp_edge; });
            }
            else {
                // Set the origin edge to the through_edge.
                for (auto e : origin.edges) {
                    if (e.id == through_edge) {
                        origin.edges.clear();
                        origin.edges.push_back(e);
                        break;
                    }
                }
            }
        }
    }

    std::list<valhalla::odin::TripPath> thor_worker_t::path_depart_at(const std::vector<valhalla::midgard::PointLL>& points, const std::string &costing, const boost::optional<int> &date_time_type) {
        std::vector<baldr::PathLocation> correlated;
        for (const auto& point : points) {
            correlated.push_back(valhalla::loki::Search(point, reader, cost->GetEdgeFilter(), cost->GetNodeFilter()));
        }

        //get time for start of request
        auto s = std::chrono::system_clock::now();
        bool prior_is_node = false;
        std::vector<baldr::PathLocation> through_loc;
        baldr::GraphId through_edge;
        std::vector<thor::PathInfo> path_edges;
        std::string origin_date_time, dest_date_time;

        std::list<valhalla::odin::TripPath> trippaths;
        baldr::PathLocation& last_break_origin = correlated[0];
        for (auto path_location = ++correlated.cbegin(); path_location != correlated.cend(); ++path_location) {
            auto origin = *std::prev(path_location);
            auto destination = *path_location;

            if (date_time_type && (*date_time_type == 0 || *date_time_type == 1) &&
                !dest_date_time.empty() && origin.stoptype_ == baldr::Location::StopType::BREAK)
                origin.date_time_ = dest_date_time;

            // Through edge is valid if last destination was "through"
            if (through_edge.Is_Valid()) {
                update_origin(origin, prior_is_node, through_edge);
            }
            else {
                last_break_origin = origin;
            }

            // Get the algorithm type for this location pair
            thor::PathAlgorithm* path_algorithm = get_path_algorithm(costing,
                origin, destination);

            // Get best path
            if (path_edges.size() == 0) {
                get_path(path_algorithm, origin, destination, path_edges);
                if (path_edges.size() == 0) {
                    throw std::runtime_error("No path could be found for input");
                }

                if (date_time_type && *date_time_type == 0 && origin_date_time.empty() &&
                    origin.stoptype_ == baldr::Location::StopType::BREAK)
                    last_break_origin.date_time_ = origin.date_time_;
            }
            else {
                // Get the path in a temporary vector
                std::vector<thor::PathInfo> temp_path;
                get_path(path_algorithm, origin, destination, temp_path);
                if (temp_path.size() == 0) {
                    throw std::runtime_error("No path could be found for input");
                }

                if (date_time_type && *date_time_type == 0 && origin_date_time.empty() &&
                    origin.stoptype_ == baldr::Location::StopType::BREAK)
                    last_break_origin.date_time_ = origin.date_time_;

                // Append the temp_path edges to path_edges, adding the elapsed
                // time from the end of the current path. If continuing along the
                // same edge, remove the prior so we do not get a duplicate edge.
                uint32_t t = path_edges.back().elapsed_time;
                if (temp_path.front().edgeid == path_edges.back().edgeid) {
                    path_edges.pop_back();
                }
                for (auto edge : temp_path) {
                    edge.elapsed_time += t;
                    path_edges.emplace_back(edge);
                }
            }

            // Build trip path for this leg and add to the result if this
            // location is a BREAK or if this is the last location
            if (destination.stoptype_ == baldr::Location::StopType::BREAK ||
                path_location == --correlated.cend()) {
                // Form output information based on path edges
                auto trip_path = thor::TripPathBuilder::Build(reader, path_edges,
                    last_break_origin, destination, through_loc);

                if (date_time_type) {
                    origin_date_time = *last_break_origin.date_time_;
                    dest_date_time = *destination.date_time_;
                }

                // The protobuf path
                trippaths.emplace_back(std::move(trip_path));

                // Clear path edges and set through edge to invalid
                path_edges.clear();
                through_edge = baldr::GraphId();
            }
            else {
                // This is a through location. Save last edge as the through_edge
                prior_is_node = false;
                for (const auto& e : origin.edges) {
                    if (e.id == path_edges.back().edgeid) {
                        prior_is_node = e.begin_node() || e.end_node();
                        break;
                    }
                }
                through_edge = path_edges.back().edgeid;

                // Add to list of through locations for this leg
                through_loc.emplace_back(destination);
            }

            // If we have another one coming we need to clear
            if (--correlated.cend() != path_location)
                path_algorithm->Clear();
        }

        return trippaths;
    }
} }

namespace carto {

    ValhallaOfflineRoutingService::ValhallaOfflineRoutingService(const std::string& path) :
        _path(path)
    {
    }

    ValhallaOfflineRoutingService::~ValhallaOfflineRoutingService() {
    }

    std::shared_ptr<RoutingResult> ValhallaOfflineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        EPSG3857 epsg3857;
        std::shared_ptr<Projection> proj = request->getProjection();
        
        std::list<valhalla::odin::TripPath> tripPaths;
        try {
            std::vector<valhalla::midgard::PointLL> points;
            for (const MapPos& pos : request->getPoints()) {
                MapPos posWgs84 = proj->toWgs84(pos);
                points.emplace_back(static_cast<float>(posWgs84.getX()), static_cast<float>(posWgs84.getY()));
            }
            
            std::string costing = "pedestrian";
            valhalla::thor::thor_worker_t worker(_path, costing);
            tripPaths = worker.path_depart_at(points, costing, boost::optional<int>());
        }
        catch (const std::exception& ex) {
            throw GenericException("Exception while calculating route", ex.what());
        }

        std::vector<MapPos> points;
        std::vector<MapPos> epsg3857Points;
        std::vector<RoutingInstruction> instructions;

        for (auto& tripPath : tripPaths) {
            valhalla::odin::DirectionsOptions directionsOptions;
            valhalla::odin::DirectionsBuilder directions;
            valhalla::odin::TripDirections tripDirections;
            try {
                tripDirections = directions.Build(directionsOptions, tripPath);

                std::vector<valhalla::midgard::PointLL> shape = valhalla::midgard::decode<std::vector<PointLL> >(tripDirections.shape());

                std::size_t pointIndex = points.size();
                for (int i = 0; i < tripDirections.maneuver_size(); i++) {
                    const valhalla::odin::TripDirections_Maneuver& maneuver = tripDirections.maneuver(i);
                    for (int j = maneuver.begin_shape_index(); j <= maneuver.end_shape_index(); j++) {
                        const valhalla::midgard::PointLL& point = shape.at(j);
                        epsg3857Points.push_back(epsg3857.fromLatLong(point.lat(), point.lng()));
                        points.push_back(proj->fromLatLong(point.lat(), point.lng()));
                    }
                }

                RoutingAction::RoutingAction action = RoutingAction::ROUTING_ACTION_NO_TURN;

                instructions.emplace_back(action, pointIndex, "", 0, 0, 0, 0);
            }
            catch (const std::exception& ex) {
                throw GenericException("Exception while translating route", ex.what());
            }
        }

        return std::make_shared<RoutingResult>(proj, points, instructions);
    }

}

#endif
