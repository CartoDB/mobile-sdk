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
     * An online routing service that uses MapZen Valhalla routing service.
     * As the class connects to an external (non-CARTO) service, this class is provided "as-is",   
     * future changes from the service provider may not be compatible with the implementation.
     * Be sure to read the Terms and Conditions of your Valhalla service provider to see if the
     * service is available for your application.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
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
         * @return The current routing profile. Can be either "auto", "auto_shorter", "bicycle", "bus", "hov", "pedestrian" or "multimodal". The default is "pedestrian".
         */
        std::string getProfile() const;
        /**
         * Sets the current routing profile.
         * @param profile The new profile. Can be either "auto", "auto_shorter", "bicycle", "bus", "hov", "pedestrian" or "multimodal".
         */
        void setProfile(const std::string& profile);

        /**
         * Returns the custom backend service URL.
         * @return The custom backend service URL. If this is not defined, an empty string is returned.
         */
        std::string getCustomServiceURL() const;
        /**
         * Sets the custom backend service URL. 
         * The custom URL may contain tag "{api_key}" which will be substituted with the set API key.
         * @param serviceURL The custom backend service URL to use. If this is empty, then the default service is used.
         */
        void setCustomServiceURL(const std::string& serviceURL);

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    private:
        static const std::string MAPZEN_SERVICE_URL;

        const std::string _apiKey;

        std::string _profile;

        std::string _serviceURL;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
