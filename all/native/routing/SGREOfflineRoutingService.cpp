#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "SGREOfflineRoutingService.h"
#include "components/Exceptions.h"
#include "geometry/FeatureCollection.h"
#include "geometry/GeoJSONGeometryWriter.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <limits>

#include <sgre/Graph.h>
#include <sgre/GraphBuilder.h>
#include <sgre/Query.h>
#include <sgre/Result.h>
#include <sgre/Rule.h>
#include <sgre/Instruction.h>
#include <sgre/RouteFinder.h>

namespace carto {

    SGREOfflineRoutingService::SGREOfflineRoutingService(const Variant& geoJSON, const Variant& config) :
        RoutingService(),
        _featureData(geoJSON.toPicoJSON()),
        _config(config.toPicoJSON()),
        _profile(),
        _routingParameters(),
        _cachedRouteFinder(),
        _mutex()
    {
    }

    SGREOfflineRoutingService::SGREOfflineRoutingService(const std::shared_ptr<Projection>& projection, const std::shared_ptr<FeatureCollection>& featureCollection, const Variant& config) :
        RoutingService(),
        _featureData(),
        _config(config.toPicoJSON()),
        _profile(),
        _routingParameters(),
        _cachedRouteFinder(),
        _mutex()
    {
        if (!featureCollection) {
            throw NullArgumentException("Null featureCollection");
        }

        GeoJSONGeometryWriter geometryWriter;
        geometryWriter.setSourceProjection(projection);
        geometryWriter.setZ(true);
        std::string err = picojson::parse(_featureData, geometryWriter.writeFeatureCollection(featureCollection));
        if (!err.empty()) {
            throw GenericException("Error while serializing feature data", err);
        }
    }

    SGREOfflineRoutingService::~SGREOfflineRoutingService() {
    }

    float SGREOfflineRoutingService::getRoutingParameter(const std::string& param) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _routingParameters.find(param);
        if (it != _routingParameters.end()) {
            return it->second;
        }
        return std::numeric_limits<float>::quiet_NaN();
    }

    void SGREOfflineRoutingService::setRoutingParameter(const std::string& param, float value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _routingParameters[param] = value;
    }

    std::string SGREOfflineRoutingService::getProfile() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _profile;
    }

    void SGREOfflineRoutingService::setProfile(const std::string& profile) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (profile != _profile) {
            _profile = profile;
            _cachedRouteFinder.reset();
        }
    }

    std::shared_ptr<RouteMatchingResult> SGREOfflineRoutingService::matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const {
        throw GenericException("matchRoute not implemented for this RoutingService");
    }

    std::shared_ptr<RoutingResult> SGREOfflineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::lock_guard<std::mutex> lock(_mutex);
        if (!_cachedRouteFinder) {
            try {
                sgre::RuleList ruleList;
                if (_config.contains("rules")) {
                    ruleList = sgre::RuleList::parse(_config.get("rules"));
                }
                ruleList.filter(_profile);
                sgre::GraphBuilder graphBuilder(std::move(ruleList));
                graphBuilder.importGeoJSON(_featureData);
                _cachedRouteFinder = sgre::RouteFinder::create(graphBuilder.build(), _config);
            }
            catch (const std::exception& ex) {
                throw GenericException("Failed to create routing graph", ex.what());
            }
        }

        std::shared_ptr<sgre::RouteFinder> routeFinder = _cachedRouteFinder;
        routeFinder->setParameters(_routingParameters);

        std::shared_ptr<Projection> proj = request->getProjection();
        EPSG3857 epsg3857;

        std::size_t totalPoints = 0;
        std::size_t totalInstructions = 0;
        std::vector<sgre::Result> results;
        for (std::size_t i = 1; i < request->getPoints().size(); i++) {
            MapPos p0 = proj->toWgs84(request->getPoints()[i - 1]);
            double z0 = request->getPoints()[i - 1].getZ();
            MapPos p1 = proj->toWgs84(request->getPoints()[i]);
            double z1 = request->getPoints()[i].getZ();

            sgre::Query query(sgre::Point(p0.getX(), p0.getY(), z0), sgre::Point(p1.getX(), p1.getY(), z1));
            picojson::value filter0 = request->getPointParameter(static_cast<int>(i - 1), "geometry_tag_filter").toPicoJSON();
            if (filter0.is<picojson::object>()) {
                query.setFilter(0, filter0.get<picojson::object>());
            }
            picojson::value filter1 = request->getPointParameter(static_cast<int>(i), "geometry_tag_filter").toPicoJSON();
            if (filter1.is<picojson::object>()) {
                query.setFilter(1, filter1.get<picojson::object>());
            }

            sgre::Result result = routeFinder->find(query);
            if (result.getStatus() == sgre::Result::Status::FAILED) {
                throw GenericException("Routing failed");
            }

            totalPoints += result.getGeometry().size();
            totalInstructions += result.getInstructions().size();
            results.push_back(std::move(result));
        }

        std::vector<MapPos> points;
        points.reserve(totalPoints);
        std::vector<MapPos> epsg3857Points;
        epsg3857Points.reserve(totalPoints);
        std::vector<RoutingInstruction> instructions;
        instructions.reserve(totalInstructions);
        for (std::size_t i = 0; i < results.size(); i++) {
            const sgre::Result& result = results[i];
            if (result.getInstructions().empty()) {
                continue;
            }

            std::size_t pointIndex = points.size();
            for (const sgre::Point& pos : result.getGeometry()) {
                MapPos mapPos = proj->fromWgs84(MapPos(pos(0), pos(1)));
                points.emplace_back(mapPos.getX(), mapPos.getY(), pos(2));
                epsg3857Points.push_back(epsg3857.fromWgs84(MapPos(pos(0), pos(1))));
            }

            for (const sgre::Instruction& instr : result.getInstructions()) {
                double distance = instr.getDistance();
                double time = instr.getTime();
                std::string streetName = instr.getTag().serialize();
                Variant geometryTag = Variant::FromPicoJSON(instr.getTag());

                RoutingAction::RoutingAction action = RoutingAction::ROUTING_ACTION_NO_TURN;
                TranslateInstructionCode(static_cast<int>(instr.getType()), action);
                if (action == RoutingAction::ROUTING_ACTION_NO_TURN) {
                    if (!instructions.empty() && instructions.back().getStreetName() == streetName && instructions.back().getGeometryTag() == geometryTag) {
                        instructions.back().setTime(instructions.back().getTime() + time);
                        instructions.back().setDistance(instructions.back().getDistance() + distance);
                    }
                    continue;
                }
                if (action == RoutingAction::ROUTING_ACTION_FINISH && i + 1 < results.size()) {
                    action = RoutingAction::ROUTING_ACTION_REACH_VIA_LOCATION;
                }

                int posIndex = static_cast<int>(pointIndex + instr.getGeometryIndex());
                float turnAngle = CalculateTurnAngle(epsg3857Points, posIndex);
                float azimuth = CalculateAzimuth(epsg3857Points, posIndex);
                instructions.emplace_back(action, posIndex, streetName, turnAngle, azimuth, distance, time);
                instructions.back().setGeometryTag(geometryTag);
            }
        }

        return std::make_shared<RoutingResult>(proj, points, instructions);
    }

    float SGREOfflineRoutingService::CalculateTurnAngle(const std::vector<MapPos>& epsg3857Points, int pointIndex) {
        int pointIndex0 = pointIndex;
        while (--pointIndex0 >= 0) {
            if (epsg3857Points.at(pointIndex0) != epsg3857Points.at(pointIndex)) {
                break;
            }
        }
        int pointIndex1 = pointIndex;
        while (++pointIndex1 < static_cast<int>(epsg3857Points.size())) {
            if (epsg3857Points.at(pointIndex1) != epsg3857Points.at(pointIndex)) {
                break;
            }
        }

        float turnAngle = 0;
        if (pointIndex0 >= 0 && pointIndex1 < static_cast<int>(epsg3857Points.size())) {
            const MapPos& p0 = epsg3857Points.at(pointIndex0);
            const MapPos& p1 = epsg3857Points.at(pointIndex);
            const MapPos& p2 = epsg3857Points.at(pointIndex1);
            MapVec v1 = p1 - p0;
            MapVec v2 = p2 - p1;
            double dot = v1.dotProduct(v2) / v1.length() / v2.length();
            turnAngle = static_cast<float>(std::acos(std::max(-1.0, std::min(1.0, dot))) * Const::RAD_TO_DEG);
        }
        return turnAngle;
    }
    
    float SGREOfflineRoutingService::CalculateAzimuth(const std::vector<MapPos>& epsg3857Points, int pointIndex) {
        int step = 1;
        for (int i = pointIndex; i >= 0; i += step) {
            if (i + 1 >= static_cast<int>(epsg3857Points.size())) {
                step = -1;
                continue;
            }
            const MapPos& p0 = epsg3857Points.at(i);
            const MapPos& p1 = epsg3857Points.at(i + 1);
            MapVec v = p1 - p0;
            if (v.length() > 0) {
                float angle = static_cast<float>(std::atan2(v.getY(), v.getX()) * Const::RAD_TO_DEG);
                float azimuth = 90 - angle;
                return (azimuth < 0 ? azimuth + 360 : azimuth);
            }
        }
        return std::numeric_limits<float>::quiet_NaN();
    }
    
    bool SGREOfflineRoutingService::TranslateInstructionCode(int instructionCode, RoutingAction::RoutingAction& action) {
        switch (static_cast<sgre::Instruction::Type>(instructionCode)) {
        case sgre::Instruction::Type::HEAD_ON:
            action = RoutingAction::ROUTING_ACTION_HEAD_ON;
            break;
        case sgre::Instruction::Type::GO_STRAIGHT:
            action = RoutingAction::ROUTING_ACTION_GO_STRAIGHT;
            break;
        case sgre::Instruction::Type::TURN_RIGHT:
            action = RoutingAction::ROUTING_ACTION_TURN_RIGHT;
            break;
        case sgre::Instruction::Type::TURN_LEFT:
            action = RoutingAction::ROUTING_ACTION_TURN_LEFT;
            break;
        case sgre::Instruction::Type::REACHED_YOUR_DESTINATION:
            action = RoutingAction::ROUTING_ACTION_FINISH;
            break;
        case sgre::Instruction::Type::GO_UP:
            action = RoutingAction::ROUTING_ACTION_GO_UP;
            break;
        case sgre::Instruction::Type::GO_DOWN:
            action = RoutingAction::ROUTING_ACTION_GO_DOWN;
            break;
        case sgre::Instruction::Type::WAIT:
            action = RoutingAction::ROUTING_ACTION_WAIT;
            break;
        default:
            Log::Infof("SGREOfflineRoutingService::TranslateInstructionCode: ignoring instruction %d", instructionCode);
            return false;
        }
        return true;
    }

}

#endif
