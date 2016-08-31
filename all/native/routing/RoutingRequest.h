/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTINGREQUEST_H_
#define _CARTO_ROUTINGREQUEST_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"

#include <memory>
#include <vector>

namespace carto {
    class Projection;

    /**
     * A class that defines required attributes for routing (end points, etc).
     */
    class RoutingRequest {
    public:
        /**
         * Constructs a new RoutingRequest instance from projection and via points.
         * @param projection The projection of the points.
         * @param points The list of points that the route must pass. Must contains at least 2 elements.
         */
        RoutingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points);
        virtual ~RoutingRequest();

        /**
         * Returns the projection of the points in the request.
         * @return The projection of the request.
         */
        const std::shared_ptr<Projection>& getProjection() const;
        /**
         * Returns the point list of the request.
         * @return The point list of the request.
         */
        const std::vector<MapPos>& getPoints() const;
        
    private:
        std::shared_ptr<Projection> _projection;
        std::vector<MapPos> _points;
    };
    
}

#endif

#endif
