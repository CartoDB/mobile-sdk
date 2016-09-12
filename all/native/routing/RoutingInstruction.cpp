#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingInstruction.h"

#include <sstream>

namespace carto {

    RoutingInstruction::RoutingInstruction() :
        _action(RoutingAction::ROUTING_ACTION_NO_TURN),
        _pointIndex(-1),
        _streetName(),
        _turnAngle(0),
        _azimuth(0),
        _distance(0),
        _time(0)
    {
    }

    RoutingInstruction::RoutingInstruction(RoutingAction::RoutingAction action, int pointIndex, const std::string& streetName, float turnAngle, float azimuth, double distance, double time) :
        _action(action),
        _pointIndex(pointIndex),
        _streetName(streetName),
        _turnAngle(turnAngle),
        _azimuth(azimuth),
        _distance(distance),
        _time(time)
    {
    }

    RoutingInstruction::~RoutingInstruction() {
    }

    RoutingAction::RoutingAction RoutingInstruction::getAction() const {
        return _action;
    }

    int RoutingInstruction::getPointIndex() const {
        return _pointIndex;
    }

    const std::string& RoutingInstruction::getStreetName() const {
        return _streetName;
    }

    float RoutingInstruction::getTurnAngle() const {
        return _turnAngle;
    }

    float RoutingInstruction::getAzimuth() const {
        return _azimuth;
    }

    double RoutingInstruction::getDistance() const {
        return _distance;
    }
    
    void RoutingInstruction::setDistance(double distance) {
        _distance = distance;
    }

    double RoutingInstruction::getTime() const {
        return _time;
    }
    
    void RoutingInstruction::setTime(double time) {
        _time = time;
    }

    std::string RoutingInstruction::toString() const {
        std::string actionName;
        switch (_action) {
            case RoutingAction::ROUTING_ACTION_HEAD_ON:
                actionName = "Head on";
                break;
            case RoutingAction::ROUTING_ACTION_FINISH:
                actionName = "Finish";
                break;
            case RoutingAction::ROUTING_ACTION_NO_TURN:
                actionName = "No turn";
                break;
            case RoutingAction::ROUTING_ACTION_GO_STRAIGHT:
                actionName = "Go straight";
                break;
            case RoutingAction::ROUTING_ACTION_TURN_RIGHT:
                actionName = "Turn right";
                break;
            case RoutingAction::ROUTING_ACTION_UTURN:
                actionName = "U turn";
                break;
            case RoutingAction::ROUTING_ACTION_TURN_LEFT:
                actionName = "Turn left";
                break;
            case RoutingAction::ROUTING_ACTION_REACH_VIA_LOCATION:
                actionName = "Reach via location";
                break;
            case RoutingAction::ROUTING_ACTION_ENTER_ROUNDABOUT:
                actionName = "Enter roundabout";
                break;
            case RoutingAction::ROUTING_ACTION_LEAVE_ROUNDABOUT:
                actionName = "Leave roundabout";
                break;
            case RoutingAction::ROUTING_ACTION_STAY_ON_ROUNDABOUT:
                actionName = "Stay on roundabout";
                break;
            case RoutingAction::ROUTING_ACTION_START_AT_END_OF_STREET:
                actionName = "Start at end of street";
                break;
            case RoutingAction::ROUTING_ACTION_ENTER_AGAINST_ALLOWED_DIRECTION:
                actionName = "Enter against allowed direction";
                break;
            case RoutingAction::ROUTING_ACTION_LEAVE_AGAINST_ALLOWED_DIRECTION:
                actionName = "Leave against allowed direction";
                break;
        }

        std::stringstream ss;
        ss << "RoutingInstruction [action=" << actionName;
        if (!_streetName.empty()) {
            ss << ", streetName=" << _streetName;
        }
        ss << ", azimuth=" << _azimuth;
        if (_turnAngle != 0) {
            ss << ", turnAngle=" << _turnAngle;
        }
        if (_distance != 0) {
            ss << ", distance=" << _distance;
        }
        if (_time != 0) {
            ss << ", time=" << _time;
        }
        ss << "]";
        return ss.str();
    }

}

#endif
