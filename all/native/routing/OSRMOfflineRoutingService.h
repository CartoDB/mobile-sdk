/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_OSRMOFFLINEROUTINGSERVICE_H_
#define _CARTO_OSRMOFFLINEROUTINGSERVICE_H_

#include "routing/RoutingService.h"

#include <memory>
#include <string>

namespace carto {
    namespace Routing {
        class RouteFinder;
    }

    /**
     * An offline routing service that uses Carto-specific routing 
     * database file created from OSRM prepared routing files.
     */
    class OSRMOfflineRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new OSRMOfflineRoutingService instance given database file.
         * @param path The full path to the database file.
         * @throws std::runtime_error If the database file could not be opened or read.
         */
        OSRMOfflineRoutingService(const std::string& path);
        virtual ~OSRMOfflineRoutingService();

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    protected:
        std::shared_ptr<Routing::RouteFinder> _routeFinder;
    };
    
}

#endif
