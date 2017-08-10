/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALHALLAOFFLINEROUTINGSERVICE_H_
#define _CARTO_VALHALLAOFFLINEROUTINGSERVICE_H_

#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "routing/RoutingService.h"

#include <memory>
#include <mutex>
#include <string>

namespace sqlite3pp {
    class database;
}

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
        explicit ValhallaOfflineRoutingService(const std::string& path);
        virtual ~ValhallaOfflineRoutingService();

        /**
         * Returns the current routing profile.
         * @return The current routing profile. Can be either "car", "auto", "auto_shorter", "bus", "bicycle", "pedestrian" or "truck". The default is "pedestrian".
         */
        std::string getProfile() const;
        /**
         * Sets the current routing profile.
         * @param profile The new profile. Can be either "car", "auto", "auto_shorter", "bus", "bicycle", "pedestrian" or "truck".
         */
        void setProfile(const std::string& profile);

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    private:
        std::shared_ptr<sqlite3pp::database> _database;
        std::string _profile;
        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
