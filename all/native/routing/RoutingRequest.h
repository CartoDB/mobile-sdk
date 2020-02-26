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
#include <mutex>
#include <vector>
#include <map>

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

        /**
         * Returns the geometry tag filter list of the request.
         * @return The geometry tag filter list of the request.
         */
        std::vector<Variant> getGeometryTagFilters() const;
        /**
         * Sets the geometry tag filter list for the request.
         * @param filters The new filter list for the request.
         */
        void setGeometryTagFilters(const std::vector<Variant>& filters);

        /**
         * Returns the custom parameters of the request.
         * @return The custom parameters of the request.
         */
        std::map<std::string, Variant> getCustomParameters() const;
        /**
         * Sets the custom parameters for the the request.
         * @param customParams The map of custom parameters to set.
         */
        void setCustomParameters(const std::map<std::string, Variant>& customParams);

        /**
         * Creates a string representation of this request object, useful for logging.
         * @return The string representation of this request object.
         */
        std::string toString() const;
        
    private:
        const std::shared_ptr<Projection> _projection;
        const std::vector<MapPos> _points;

        std::vector<Variant> _filters;
        std::map<std::string, Variant> _customParams;
        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
