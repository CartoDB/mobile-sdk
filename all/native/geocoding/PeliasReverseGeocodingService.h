/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PELIASREVERSEGEOCODINGSERVICE_H_
#define _CARTO_PELIASREVERSEGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT)

#include "geocoding/ReverseGeocodingService.h"
#include "network/HTTPClient.h"

namespace carto {

    /**
     * A reverse geocoding service that uses Mapzen Pelias geocoder.
     */
    class PeliasReverseGeocodingService : public ReverseGeocodingService {
    public:
        /**
         * Constructs a new instance of the OSMOfflineReverseGeocodingService given path to the geocoding database.
         * @param apiKey The API key to use registered with Mapzen.
         */
        explicit PeliasReverseGeocodingService(const std::string& apiKey);
        virtual ~PeliasReverseGeocodingService();

        /**
         * Returns the search radius (in meters).
         * @return The search radius in meters.
         */
        float getSearchRadius() const;
        /**
         * Sets the search radius (in meters).
         * The default search radius is 100 meters.
         * @param radius The new search radius in meters.
         */
        void setSearchRadius(float radius);

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const;

    protected:
        static const float DEFAULT_SEARCH_RADIUS;
        static const std::string PELIAS_REVERSE_URL;

        std::string _apiKey;
        HTTPClient _httpClient;

        float _searchRadius;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
