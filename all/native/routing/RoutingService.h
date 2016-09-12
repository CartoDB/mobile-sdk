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

#include <memory>

namespace carto {

    /**
     * An abstract base class for routing services (either online or offline).
     */
    class RoutingService {
    public:
        virtual ~RoutingService();

        /**
         * Calculates routing result (path) based on routing result.
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
