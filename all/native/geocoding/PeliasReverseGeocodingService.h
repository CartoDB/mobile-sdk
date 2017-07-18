/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PELIASREVERSEGEOCODINGSERVICE_H_
#define _CARTO_PELIASREVERSEGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT)

#include "geocoding/ReverseGeocodingService.h"

namespace carto {

    /**
     * A reverse geocoding service that uses Mapzen Pelias geocoder.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class PeliasReverseGeocodingService : public ReverseGeocodingService {
    public:
        /**
         * Constructs a new instance of the OSMOfflineReverseGeocodingService given path to the geocoding database.
         * @param apiKey The API key to use registered with Mapzen.
         */
        explicit PeliasReverseGeocodingService(const std::string& apiKey);
        virtual ~PeliasReverseGeocodingService();

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const;

    protected:
        static const std::string PELIAS_REVERSE_URL;

        std::string _apiKey;
    };
    
}

#endif

#endif
