/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_REVERSEGEOCODINGSERVICE_H_
#define _CARTO_REVERSEGEOCODINGSERVICE_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include "geocoding/ReverseGeocodingRequest.h"
#include "geocoding/GeocodingResult.h"

#include <memory>
#include <vector>

namespace carto {

    /**
     * An abstract base class for reverse geocoding services.
     */
    class ReverseGeocodingService {
    public:
        virtual ~ReverseGeocodingService();

        /**
         * Calculates matching addresses from the specified reverse geocoding request.
         * @param request The reverse geocoding request to use.
         * @result The list of matching geocoding results, sorted by descending ranks.
         * @throws std::runtime_error If IO error occured during the calculation.
         */
        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const = 0;

    protected:
        /**
         * The default constructor.
         */
        ReverseGeocodingService();
    };
    
}

#endif

#endif
