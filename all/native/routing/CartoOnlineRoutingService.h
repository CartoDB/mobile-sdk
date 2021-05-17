/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOONLINEROUTINGSERVICE_H_
#define _CARTO_CARTOONLINEROUTINGSERVICE_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "routing/RoutingService.h"

#include <memory>
#include <string>

namespace carto {
    class Projection;

    /**
     * An online routing service that connects to Carto-provided routing server.
     * Routing and route matching perform network requests and must be executed in non-UI background thread.
     * Note: this service is not actively maintained and will be removed in future versions.
     * @deprecated
     */
    class CartoOnlineRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new CartoOnlineRoutingService instance given service source.
         * @param source The source id of the service.
         */
        explicit CartoOnlineRoutingService(const std::string& source);
        virtual ~CartoOnlineRoutingService();

        virtual std::string getProfile() const;
        virtual void setProfile(const std::string& profile);

        virtual std::shared_ptr<RouteMatchingResult> matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const;

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    protected:
        static const std::string ROUTING_SERVICE_TEMPLATE;

        const std::string _source;
    };
    
}

#endif

#endif
