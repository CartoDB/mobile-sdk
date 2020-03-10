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

        virtual bool isAutocomplete() const;
        virtual void setAutocomplete(bool autocomplete);

        virtual std::string getLanguage() const;
        virtual void setLanguage(const std::string& lang);

        virtual int getMaxResults() const;
        virtual void setMaxResults(int maxResults);

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const;

    protected:
        std::shared_ptr<geocoding::Geocoder> _geocoder;
    };
    
}

#endif

#endif
