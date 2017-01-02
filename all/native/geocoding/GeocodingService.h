/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGSERVICE_H_
#define _CARTO_GEOCODINGSERVICE_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include "routing/GeocodingRequest.h"
#include "routing/GeocodingResult.h"

#include <memory>

namespace carto {

    class GeocodingService {
    public:
        virtual ~GeocodingService();

        virtual std::vector<GeocodingResult> calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const = 0;

    protected:
        /**
         * The default constructor.
         */
        GeocodingService();
    };
    
}

#endif

#endif
