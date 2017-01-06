/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_OSMOFFILINEGEOCODINGSERVICE_H_
#define _CARTO_OSMOFFILINEGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "geocoding/GeocodingService.h"
#include "projections/Projection.h"

#include <memory>

namespace sqlite3pp {
    class database;
}

namespace carto {
    namespace geocoding {
        class Geocoder;
    }

    class OSMOfflineGeocodingService : public GeocodingService {
    public:
        OSMOfflineGeocodingService(const std::shared_ptr<Projection>& projection, const std::string& path);
        virtual ~OSMOfflineGeocodingService();

        bool isAutocomplete() const;
        void setAutocomplete(bool autocomplete);

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const;

    protected:
        std::shared_ptr<Projection> _projection;
        std::shared_ptr<sqlite3pp::database> _database;
        std::shared_ptr<geocoding::Geocoder> _geocoder;
    };
    
}

#endif

#endif
