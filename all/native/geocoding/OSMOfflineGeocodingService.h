/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_OSMOFFILINEGEOCODINGSERVICE_H_
#define _CARTO_OSMOFFILINEGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "geocoding/GeocodingService.h"

namespace carto {
    namespace geocoding {
        class Geocoder;
    }

    /**
     * A geocoding service that uses custom geocoding database files.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class OSMOfflineGeocodingService : public GeocodingService {
    public:
        /**
         * Constructs a new instance of the OSMOfflineGeocodingService given path to the geocoding database.
         * @param path The full path to the geocoding database file.
         * @throws std::runtime_error If the database file could not be opened or read.
         */
        explicit OSMOfflineGeocodingService(const std::string& path);
        virtual ~OSMOfflineGeocodingService();

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

        /**
         * Returns the language of the expected results.
         * @return The language of the expected results. As ISO 639-1 code or empty string.
         */
        std::string getLanguage() const;
        /**
         * Sets the language of the expected results.
         * @param lang The language to use as ISO 639-1 code. Empty string can be used for default language.
         */
        void setLanguage(const std::string& lang);

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const;

    protected:
        std::shared_ptr<geocoding::Geocoder> _geocoder;
    };
    
}

#endif

#endif
