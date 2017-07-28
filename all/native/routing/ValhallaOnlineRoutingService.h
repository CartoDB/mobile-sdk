/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALHALLAONLINEROUTINGSERVICE_H_
#define _CARTO_VALHALLAONLINEROUTINGSERVICE_H_

#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT)

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
    class ValhallaOnlineRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new ValhallaOnlineRoutingService instance given database file.
         * @param apiKey The API key to use registered with Mapzen.
         */
        explicit ValhallaOnlineRoutingService(const std::string& apiKey);
        virtual ~ValhallaOnlineRoutingService();

        /**
         * Returns the current routing profile.
         * @return The current routing profile. Can be either "car", "auto", "auto_shorter", "bus", "bicycle", "pedestrian" or "truck". The default is "pedestrian".
         */
        std::string getProfile() const;
        /**
         * Sets the current routing profile.
         * @profile The new profile. Can be either "car", "auto", "auto_shorter", "bus", "bicycle", "pedestrian" or "truck".
         */
        void setProfile(const std::string& profile);

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    private:
        static const std::string VALHALLA_ROUTING_URL;

        const std::string _apiKey;

        std::string _profile;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
