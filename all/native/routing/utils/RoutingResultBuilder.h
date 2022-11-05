/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTINGRESULTBUILDER_H_
#define _CARTO_ROUTINGRESULTBUILDER_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"
#include "routing/RoutingResult.h"
#include "routing/utils/RoutingInstructionBuilder.h"

#include <memory>
#include <string>
#include <vector>
#include <list>

namespace carto {
    class Projection;

    class RoutingResultBuilder {
    public:
        explicit RoutingResultBuilder(const std::shared_ptr<Projection>& proj);

        int addPoints(const std::vector<MapPos>& points);

        RoutingInstructionBuilder& addInstruction(RoutingAction::RoutingAction action, int pointIndex);

        std::shared_ptr<RoutingResult> buildRoutingResult() const;
        
    private:
        float calculateTurnAngle(int pointIndex) const;

        float calculateAzimuth(int pointIndex) const;

        std::string calculateDirection(float azimuth) const;

        std::string calculateDistance(double distance) const;

        std::string calculateInstruction(const RoutingInstructionBuilder& instr) const;

        const std::shared_ptr<Projection> _projection;
        std::vector<MapPos> _points;
        std::list<RoutingInstructionBuilder> _instructions;
    };
    
}

#endif

#endif
