/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PELIASONLINEREVERSEGEOCODINGSERVICE_H_
#define _CARTO_PELIASONLINEREVERSEGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT)

#include "geocoding/ReverseGeocodingService.h"

namespace carto {

    /**
     * An online reverse geocoding service that uses Mapzen Pelias geocoder.
     * As the class connects to an external (non-CARTO) service, this class is provided "as-is",   
     * future changes from the service provider may not be compatible with the implementation.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class PeliasOnlineReverseGeocodingService : public ReverseGeocodingService {
    public:
        /**
         * Constructs a new instance of the PeliasOnlineReverseGeocodingService given API key.
         * @param apiKey The API key to use (registered with Mapzen).
         */
        explicit PeliasOnlineReverseGeocodingService(const std::string& apiKey);
        virtual ~PeliasOnlineReverseGeocodingService();

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

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const;

    protected:
        static const std::string MAPZEN_SERVICE_URL;

        std::string _serviceURL;

        const std::string _apiKey;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
