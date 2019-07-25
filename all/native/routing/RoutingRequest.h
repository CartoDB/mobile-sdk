/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTINGREQUEST_H_
#define _CARTO_ROUTINGREQUEST_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"
#include "core/Variant.h"

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
        /**
         * Constructs a new RoutingRequest instance from projection, via points and via filters.
         * @param projection The projection of the points.
         * @param points The list of points that the route must pass. Must contains at least 2 elements.
         * @param filters The list of attribute values each corresponding point has to match. Can be empty of must be contains same number of elements as points.
         */
        RoutingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points, const std::vector<Variant>& filters);
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
        /**
         * Returns the filter list of the request.
         * @return The filter list of the request.
         */
        const std::vector<Variant>& getFilters() const;

        /**
         * Creates a string representation of this request object, useful for logging.
         * @return The string representation of this request object.
         */
        std::string toString() const;
        
    private:
        std::shared_ptr<Projection> _projection;
        std::vector<MapPos> _points;
        std::vector<Variant> _filters;
    };
    
}

#endif

#endif
