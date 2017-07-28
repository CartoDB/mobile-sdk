/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PELIASGEOCODINGSERVICE_H_
#define _CARTO_PELIASGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT)

#include "geocoding/GeocodingService.h"

namespace carto {

    /**
     * A geocoding service that uses Mapzen Pelias geocoder.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class PeliasGeocodingService : public GeocodingService {
    public:
        /**
         * Constructs a new instance of the PeliasGeocodingService given API key.
         * @param apiKey The API key to use registered with Mapzen.
         */
        explicit PeliasGeocodingService(const std::string& apiKey);
        virtual ~PeliasGeocodingService();

        /**
         * Returns the autocomplete flag of the service.
         * @return The autocomplete flag of the service.
         */
        bool isAutocomplete() const;
        /**
         * Sets the autocomplete flag of the service.
         * By default this flag is off.
         * @param autocomplete The new value for autocomplete flag.
         */
        void setAutocomplete(bool autocomplete);

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const;

    protected:
        static const std::string PELIAS_AUTOCOMPLETE_URL;
        static const std::string PELIAS_SEARCH_URL;

        const std::string _apiKey;

        bool _autocomplete;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
