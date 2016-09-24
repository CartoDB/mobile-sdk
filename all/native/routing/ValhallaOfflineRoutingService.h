/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALHALLAOFFLINEROUTINGSERVICE_H_
#define _CARTO_VALHALLAOFFLINEROUTINGSERVICE_H_

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "routing/RoutingService.h"

#include <memory>
#include <string>

namespace carto {

    /**
     * An offline routing service that uses Valhalla routing tiles.
     */
    class ValhallaOfflineRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new ValhallaOfflineRoutingService instance given database file.
         * @param path The full path to the database file.
         * @throws std::runtime_error If the database file could not be opened or read.
         */
        ValhallaOfflineRoutingService(const std::string& path);
        virtual ~ValhallaOfflineRoutingService();

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    private:
        std::string _path;
    };
    
}

#endif

#endif
