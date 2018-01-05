/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPBOXONLINEREVERSEGEOCODINGSERVICE_H_
#define _CARTO_MAPBOXONLINEREVERSEGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT)

#include "geocoding/ReverseGeocodingService.h"

namespace carto {

    /**
     * An online reverse geocoding service that uses MapBox geocoder.
     * As the class connects to an external (non-CARTO) service, this class is provided "as-is",   
     * future changes from the service provider may not be compatible with the implementation.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class MapBoxOnlineReverseGeocodingService : public ReverseGeocodingService {
    public:
        /**
         * Constructs a new instance of the MapBoxOnlineReverseGeocodingService given API key.
         * @param apiKey The API key to use registered with MapBox.
         */
        explicit MapBoxOnlineReverseGeocodingService(const std::string& apiKey);
        virtual ~MapBoxOnlineReverseGeocodingService();

        // TODO: language

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const;

    protected:
        static const std::string MAPBOX_SERVICE_URL;

        const std::string _apiKey;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
