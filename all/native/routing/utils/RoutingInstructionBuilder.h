/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTINGINSTRUCTIONBUILDER_H_
#define _CARTO_ROUTINGINSTRUCTIONBUILDER_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"
#include "core/Variant.h"
#include "routing/RoutingInstruction.h"

#include <memory>
#include <string>

namespace carto {

    class RoutingInstructionBuilder {
    public:
        RoutingInstructionBuilder();
        
        RoutingAction::RoutingAction getAction() const;
        void setAction(RoutingAction::RoutingAction action);

        int getPointIndex() const;
        void setPointIndex(int pointIndex);

        const std::string& getStreetName() const;
        void setStreetName(const std::string& streetName);

        const std::string& getInstruction() const;
        void setInstruction(const std::string& instruction);

        float getTurnAngle() const;
        void setTurnAngle(float angle);

        float getAzimuth() const;
        void setAzimuth(float azimuth);

        double getDistance() const;
        void setDistance(double distance);

        double getTime() const;
        void setTime(double time);

        const Variant& getGeometryTag() const;
        void setGeometryTag(const Variant& variant);

        RoutingInstruction buildRoutingInstruction() const;

    private:
        RoutingAction::RoutingAction _action;
        int _pointIndex;
        std::string _streetName;
        std::string _instruction;
        float _turnAngle;
        float _azimuth;
        double _distance;
        double _time;
        Variant _geometryTag;
    };
    
}

#endif

#endif
