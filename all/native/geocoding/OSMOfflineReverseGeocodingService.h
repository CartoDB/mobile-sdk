/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_OSMOFFILINEREVERSEGEOCODINGSERVICE_H_
#define _CARTO_OSMOFFILINEREVERSEGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "geocoding/ReverseGeocodingService.h"

namespace carto {
    namespace geocoding {
        class RevGeocoder;
    }

    /**
     * A reverse geocoding service that uses custom geocoding database files.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class OSMOfflineReverseGeocodingService : public ReverseGeocodingService {
    public:
        /**
         * Constructs a new instance of the OSMOfflineReverseGeocodingService given path to the geocoding database.
         * @param path The full path to the geocoding database file.
         */
        explicit OSMOfflineReverseGeocodingService(const std::string& path);
        virtual ~OSMOfflineReverseGeocodingService();

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
        std::shared_ptr<geocoding::RevGeocoder> _revGeocoder;
    };
    
}

#endif

#endif
