#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingResultBuilder.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Const.h"

#include <utility>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace carto {

    RoutingResultBuilder::RoutingResultBuilder(const std::shared_ptr<Projection>& proj, const std::string& rawResult) :
        _projection(proj),
        _rawResult(rawResult),
        _points(),
        _instructions()
    {
    }

    int RoutingResultBuilder::addPoints(const std::vector<MapPos>& points) {
        int pointIndex0 = static_cast<int>(_points.size());
        for (std::size_t i = 0; i < points.size(); i++) {
            if (i != 0 || _points.empty() || _points.back() != points[i]) {
                _points.push_back(points[i]);
            }
            if (i == 0) {
                pointIndex0 = static_cast<int>(_points.size() - 1);
            }
        }
        return pointIndex0;
    }

    RoutingInstructionBuilder& RoutingResultBuilder::addInstruction(RoutingAction::RoutingAction action, int pointIndex) {
        _instructions.emplace_back();
        RoutingInstructionBuilder& instrBuilder = _instructions.back();
        instrBuilder.setAction(action);
        instrBuilder.setPointIndex(pointIndex);
        instrBuilder.setAzimuth(std::numeric_limits<float>::quiet_NaN());
        instrBuilder.setTurnAngle(std::numeric_limits<float>::quiet_NaN());
        return instrBuilder;
    }

    std::shared_ptr<RoutingResult> RoutingResultBuilder::buildRoutingResult() const {
        std::vector<RoutingInstruction> instructions;
        instructions.reserve(_instructions.size());
        for (RoutingInstructionBuilder instrBuilder : _instructions) {
            if (std::isnan(instrBuilder.getTurnAngle())) {
                instrBuilder.setTurnAngle(calculateTurnAngle(instrBuilder.getPointIndex()));
            }
            if (std::isnan(instrBuilder.getAzimuth())) {
                instrBuilder.setAzimuth(calculateAzimuth(instrBuilder.getPointIndex()));
            }
            if (instrBuilder.getInstruction().empty()) {
                instrBuilder.setInstruction(calculateInstruction(instrBuilder));
            }
            instructions.push_back(instrBuilder.buildRoutingInstruction());
        }
        return std::make_shared<RoutingResult>(_projection, _points, std::move(instructions), _rawResult);
    }

    float RoutingResultBuilder::calculateTurnAngle(int pointIndex) const {
        EPSG3857 epsg3857;

        if (pointIndex < 0 || pointIndex >= static_cast<int>(_points.size())) {
            return 0.0f;
        }

        MapPos p1 = epsg3857.fromInternal(_projection->toInternal(_points[pointIndex]));
        MapPos p0 = p1;
        MapPos p2 = p1;

        int pointIndex0 = pointIndex;
        while (--pointIndex0 >= 0) {
            p0 = epsg3857.fromInternal(_projection->toInternal(_points[pointIndex0]));
            if (p0 != p1) {
                break;
            }
        }
        int pointIndex2 = pointIndex;
        while (++pointIndex2 < static_cast<int>(_points.size())) {
            p2 = epsg3857.fromInternal(_projection->toInternal(_points[pointIndex2]));
            if (p2 != p1) {
                break;
            }
        }

        if (pointIndex0 >= 0 && pointIndex2 < static_cast<int>(_points.size())) {
            MapVec v10 = p1 - p0;
            MapVec v21 = p2 - p1;
            double dot = v10.dotProduct(v21) / (v10.length() * v21.length());
            return static_cast<float>(std::acos(std::max(-1.0, std::min(1.0, dot))) * Const::RAD_TO_DEG);
        }
        return 0.0f;
    }

    float RoutingResultBuilder::calculateAzimuth(int pointIndex) const {
        EPSG3857 epsg3857;

        int step = 1;
        for (int i = pointIndex; i >= 0; i += step) {
            if (i + 1 >= static_cast<int>(_points.size())) {
                step = -1;
                continue;
            }

            MapPos p0 = epsg3857.fromInternal(_projection->toInternal(_points[i + 0]));
            MapPos p1 = epsg3857.fromInternal(_projection->toInternal(_points[i + 1]));
            MapVec v10 = p1 - p0;
            if (v10.length() > 0) {
                float angle = static_cast<float>(std::atan2(v10.getY(), v10.getX()) * Const::RAD_TO_DEG);
                float azimuth = 90.0f - angle;
                return (azimuth < 0 ? azimuth + 360.0f : azimuth);
            }
        }
        return std::numeric_limits<float>::quiet_NaN();
    }

    std::string RoutingResultBuilder::calculateDirection(float azimuth) const {
        static const std::vector<std::pair<float, std::string> > directions = {
            { 0.0f  , "north"     },
            { 45.0f , "northeast" },
            { 90.0f , "east"      },
            { 135.0f, "southeast" },
            { 180.0f, "south"     },
            { 225.0f, "southwest" },
            { 270.0f, "west"      },
            { 315.0f, "northwest" },
        };

        float bestDiff = std::numeric_limits<float>::infinity();
        std::string bestDirection;
        for (const auto& direction : directions) {
            float diff = std::min(std::abs(azimuth - direction.first), std::abs(azimuth - direction.first - 360.0f));
            if (diff < bestDiff) {
                bestDiff = diff;
                bestDirection = direction.second;
            }
        }
        return bestDirection;
    }

    std::string RoutingResultBuilder::calculateDistance(double distance) const {
        std::stringstream ss;
        if (distance < 1000) {
            ss << std::fixed << std::setprecision(distance >= 10 ? 0 : 1) << distance << "m";
        } else {
            ss << std::fixed << std::setprecision(distance >= 10000 ? 0 : 1) << (distance / 1000.0) << "km";
        }
        return ss.str();
    }

    std::string RoutingResultBuilder::calculateInstruction(const RoutingInstructionBuilder& instr) const {
        std::string instruction;
        std::string direction = calculateDirection(instr.getAzimuth());
        std::string streetName = instr.getStreetName();
        if (!streetName.empty()) {
            if (streetName.front() == '{' && streetName.back() == '}') {
                streetName.clear();
            }
        }

        switch (instr.getAction()) {
        case RoutingAction::ROUTING_ACTION_HEAD_ON:
            instruction = "Head on " + direction + (streetName.empty() ? std::string() : " on " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_FINISH:
            instruction = "You have reached your destination";
            break;
        case RoutingAction::ROUTING_ACTION_NO_TURN:
        case RoutingAction::ROUTING_ACTION_GO_STRAIGHT:
            instruction = "Go straight " + direction + (streetName.empty() ? std::string() : " on " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_TURN_RIGHT:
            instruction = (instr.getTurnAngle() < 30.0f ? "Bear right" : "Turn right") + (streetName.empty() ? std::string() : " onto " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_UTURN:
            instruction = "Make U turn" + (streetName.empty() ? std::string() : " onto " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_TURN_LEFT:
            instruction = (instr.getTurnAngle() < 30.0f ? "Bear left" : "Turn left") + (streetName.empty() ? std::string() : " onto " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_REACH_VIA_LOCATION:
            instruction = "You have reached your non-final destination";
            break;
        case RoutingAction::ROUTING_ACTION_ENTER_ROUNDABOUT:
            instruction = "Enter the roundabout";
            break;
        case RoutingAction::ROUTING_ACTION_LEAVE_ROUNDABOUT:
            instruction = "Exit the roundabout" + (streetName.empty() ? std::string() : " onto " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_STAY_ON_ROUNDABOUT:
            instruction = "Stay on the roundabout";
            break;
        case RoutingAction::ROUTING_ACTION_START_AT_END_OF_STREET:
            instruction = "Start at end of a street" + (streetName.empty() ? std::string() : " on " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_ENTER_AGAINST_ALLOWED_DIRECTION:
            instruction = "Enter against the allowed direction"  + (streetName.empty() ? std::string() : " on " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_LEAVE_AGAINST_ALLOWED_DIRECTION:
            instruction = "Leave against the allowed direction" + (streetName.empty() ? std::string() : " onto " + streetName);
            break;
        case RoutingAction::ROUTING_ACTION_GO_UP:
            instruction = "Go up for " + calculateDistance(instr.getDistance());
            break;
        case RoutingAction::ROUTING_ACTION_GO_DOWN:
            instruction = "Go down for " + calculateDistance(instr.getDistance());
            break;
        case RoutingAction::ROUTING_ACTION_WAIT:
            instruction = "Wait for your turn";
            break;
        case RoutingAction::ROUTING_ACTION_ENTER_FERRY:
            instruction = (streetName.empty() ? std::string("Take a ferry") : "Take the " + streetName + " ferry");
            break;
        case RoutingAction::ROUTING_ACTION_LEAVE_FERRY:
            instruction = "Leave the ferry and head on " + direction + (streetName.empty() ? std::string() : " on " + streetName);
            break;
        }
        return instruction;
    }

}

#endif
