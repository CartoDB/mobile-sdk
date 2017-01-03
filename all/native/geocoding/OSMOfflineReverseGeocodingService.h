/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_OSMOFFILINEREVERSEGEOCODINGSERVICE_H_
#define _CARTO_OSMOFFILINEREVERSEGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "geocoding/ReverseGeocodingService.h"
#include "projections/Projection.h"

#include <memory>

namespace carto {
    namespace geocoding {
        class RevGeocoder;
    }

    class OSMOfflineReverseGeocodingService : public ReverseGeocodingService {
    public:
        explicit OSMOfflineReverseGeocodingService(const std::shared_ptr<Projection>& projection);
        virtual ~OSMOfflineReverseGeocodingService();

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const;

    protected:
        std::shared_ptr<Projection> _projection;
        std::shared_ptr<geocoding::RevGeocoder> _revGeocoder;
    };
    
}

#endif

#endif
