#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingInstructionBuilder.h"

#include <sstream>

namespace carto {

    RoutingInstructionBuilder::RoutingInstructionBuilder() :
        _action(RoutingAction::ROUTING_ACTION_NO_TURN),
        _pointIndex(-1),
        _streetName(),
        _instruction(),
        _turnAngle(0),
        _azimuth(0),
        _distance(0),
        _time(0),
        _geometryTag()
    {
    }

    RoutingAction::RoutingAction RoutingInstructionBuilder::getAction() const {
        return _action;
    }

    void RoutingInstructionBuilder::setAction(RoutingAction::RoutingAction action) {
        _action = action;
    }

    int RoutingInstructionBuilder::getPointIndex() const {
        return _pointIndex;
    }

    void RoutingInstructionBuilder::setPointIndex(int pointIndex) {
        _pointIndex = pointIndex;
    }

    const std::string& RoutingInstructionBuilder::getStreetName() const {
        return _streetName;
    }

    void RoutingInstructionBuilder::setStreetName(const std::string& streetName) {
        _streetName = streetName;
    }

    const std::string& RoutingInstructionBuilder::getInstruction() const {
        return _instruction;
    }

    void RoutingInstructionBuilder::setInstruction(const std::string& instruction) {
        _instruction = instruction;
    }

    float RoutingInstructionBuilder::getTurnAngle() const {
        return _turnAngle;
    }

    void RoutingInstructionBuilder::setTurnAngle(float angle) {
        _turnAngle = angle;
    }

    float RoutingInstructionBuilder::getAzimuth() const {
        return _azimuth;
    }

    void RoutingInstructionBuilder::setAzimuth(float azimuth) {
        _azimuth = azimuth;
    }

    double RoutingInstructionBuilder::getDistance() const {
        return _distance;
    }
    
    void RoutingInstructionBuilder::setDistance(double distance) {
        _distance = distance;
    }

    double RoutingInstructionBuilder::getTime() const {
        return _time;
    }
    
    void RoutingInstructionBuilder::setTime(double time) {
        _time = time;
    }

    const Variant& RoutingInstructionBuilder::getGeometryTag() const {
        return _geometryTag;
    }

    void RoutingInstructionBuilder::setGeometryTag(const Variant& geometryTag) {
        _geometryTag = geometryTag;
    }

    RoutingInstruction RoutingInstructionBuilder::buildRoutingInstruction() const {
        return RoutingInstruction(_action, _pointIndex, _streetName, _instruction, _turnAngle, _azimuth, _distance, _time, _geometryTag);
    }

}

#endif
