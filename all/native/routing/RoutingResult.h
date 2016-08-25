/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTINGRESULT_H_
#define _CARTO_ROUTINGRESULT_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"
#include "routing/RoutingInstruction.h"

#include <memory>
#include <vector>

namespace carto {
    class Projection;

    /**
     * A class that defines list of routing actions and path geometry.
     */
    class RoutingResult {
    public:
        /**
         * Constructs a new RoutingResult instance from projection, points and instructions.
         * @param projection The projection of the routing result (same as the request).
         * @param points The point list defining the routing path. Instructions refer to this list.
         * @param instructions The turn-by-turn instruction list.
         */
        RoutingResult(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points, const std::vector<RoutingInstruction>& instructions);
        virtual ~RoutingResult();

        /**
         * Returns the projection of the points in the result.
         * @return The projection of the result.
         */
        const std::shared_ptr<Projection>& getProjection() const;
        /**
         * Returns the point list of the result. The list contains all the points the route must pass in correct order.
         * @return The point list of the path.
         */
        const std::vector<MapPos>& getPoints() const;
        /**
         * Returns the turn-by-turn instruction list.
         * @return The turn-by-turn instruction list.
         */
        const std::vector<RoutingInstruction>& getInstructions() const;

        /**
         * Returns the total distance of the path.
         * @return The total distance in meters.
         */
        double getTotalDistance() const;
        /**
         * Returns the approximate total duration of the path.
         * @return The total duration in seconds.
         */
        double getTotalTime() const;
        
    private:
        std::shared_ptr<Projection> _projection;
        std::vector<MapPos> _points;
        std::vector<RoutingInstruction> _instructions;
    };
    
}

#endif

#endif
