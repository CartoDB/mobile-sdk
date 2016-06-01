/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOOFFLINEROUTINGSERVICE_H_
#define _CARTO_CARTOOFFLINEROUTINGSERVICE_H_

#include "routing/RoutingService.h"

#include <memory>
#include <string>

namespace carto {
    namespace Routing {
        class RouteFinder;
    }

    /**
     * An offline routing service that uses Nutiteq-specific routing database file.
     */
    class CartoOfflineRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new CartoOfflineRoutingService instance given database file.
         * @param path The full path to the database file.
         */
        CartoOfflineRoutingService(const std::string& path);
        virtual ~CartoOfflineRoutingService();

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    protected:
        std::shared_ptr<Routing::RouteFinder> _routeFinder;
    };
    
}

#endif
