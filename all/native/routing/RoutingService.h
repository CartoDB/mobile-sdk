/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTINGSERVICE_H_
#define _CARTO_ROUTINGSERVICE_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "routing/RoutingRequest.h"
#include "routing/RoutingResult.h"
#include "routing/RouteMatchingRequest.h"
#include "routing/RouteMatchingResult.h"

#include <memory>

namespace carto {

    /**
     * An abstract base class for routing services (either online or offline).
     */
    class RoutingService {
    public:
        virtual ~RoutingService();

        /**
         * Returns the current routing profile.
         * @return The current routing profile.
         */
        virtual std::string getProfile() const = 0;
        /**
         * Sets the current routing profile.
         * @param profile The new routing profile. Routing profiles are dependent on specific routing service.
         */
        virtual void setProfile(const std::string& profile) = 0;

        /**
         * Matches specified points to the points on the road network.
         * @param request The matching request defining points.
         * @return The matching result or null if route matching failed.
         * @throws std::runtime_error If IO error occured during the route matching.
         */
        virtual std::shared_ptr<RouteMatchingResult> matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const = 0;

        /**
         * Calculates routing result (path) based on routing request.
         * @param request The routing request defining via points.
         * @return The result or null if routing failed.
         * @throws std::runtime_error If IO error occured during the route calculation.
         */
        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const = 0;

    protected:
        /**
         * The default constructor.
         */
        RoutingService();
    };
    
}

#endif

#endif
