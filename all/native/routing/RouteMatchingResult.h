/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTEMATCHINGRESULT_H_
#define _CARTO_ROUTEMATCHINGRESULT_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"

#include <memory>
#include <vector>

namespace carto {
    class Projection;

    /**
     * A class that defines list of matching points from road network.
     */
    class RouteMatchingResult {
    public:
        /**
         * Constructs a new RouteMatchingResult instance from projection and matched points.
         * @param projection The projection of the routing result (same as the request).
         * @param points The points corresponding to the requested points snapped to road network.
         */
        RouteMatchingResult(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points);
        virtual ~RouteMatchingResult();

        /**
         * Returns the projection of the points in the result.
         * @return The projection of the result.
         */
        const std::shared_ptr<Projection>& getProjection() const;
        /**
         * Returns the point list of the result. The list contains all the points from the request snapped to the road network.
         * @return The point list of the result.
         */
        const std::vector<MapPos>& getPoints() const;

        /**
         * Creates a string representation of this result object, useful for logging.
         * @return The string representation of this result object.
         */
        std::string toString() const;
        
    private:
        std::shared_ptr<Projection> _projection;
        std::vector<MapPos> _points;
    };
    
}

#endif

#endif
