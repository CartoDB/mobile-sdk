/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALHALLAONLINEROUTINGSERVICE_H_
#define _CARTO_VALHALLAONLINEROUTINGSERVICE_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "routing/RoutingService.h"

#include <memory>
#include <mutex>
#include <string>

namespace sqlite3pp {
    class database;
}

namespace carto {

    /**
     * An online routing service that uses MapBox Valhalla routing service.
     * As the class connects to an external (non-CARTO) service, this class is provided "as-is",   
     * future changes from the service provider may not be compatible with the implementation.
     * Routing and route matching perform network requests and must be executed in non-UI background thread.
     * Be sure to read the Terms and Conditions of your Valhalla service provider to see if the
     * service is available for your application.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class ValhallaOnlineRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new ValhallaOnlineRoutingService instance given database file.
         * @param apiKey The API key (access token) to use registered with MapBox.
         */
        explicit ValhallaOnlineRoutingService(const std::string& apiKey);
        virtual ~ValhallaOnlineRoutingService();

        /**
         * Returns the custom backend service URL.
         * @return The custom backend service URL. If this is not defined, an empty string is returned.
         */
        std::string getCustomServiceURL() const;
        /**
         * Sets the custom backend service URL. 
         * The custom URL should contain tag "{service}", it will be substituted by the SDK by the service type the SDK needs to perform ("route" or "trace_route").
         * The custom URL may also contain tag "{api_key}" which will be substituted with the set API key.
         * @param serviceURL The custom backend service URL to use. If this is empty, then the default service is used.
         */
        void setCustomServiceURL(const std::string& serviceURL);

        virtual std::string getProfile() const;
        virtual void setProfile(const std::string& profile);

        virtual std::shared_ptr<RouteMatchingResult> matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const;

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    private:
        static const std::string MAPBOX_SERVICE_URL;

        const std::string _apiKey;

        std::string _profile;

        std::string _serviceURL;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
