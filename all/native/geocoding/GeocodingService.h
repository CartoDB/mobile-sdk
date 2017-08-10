/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGSERVICE_H_
#define _CARTO_GEOCODINGSERVICE_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include "geocoding/GeocodingRequest.h"
#include "geocoding/GeocodingResult.h"

#include <memory>
#include <vector>

namespace carto {

    /**
     * An abstract base class for geocoding services.
     */
    class GeocodingService {
    public:
        virtual ~GeocodingService();

        /**
         * Calculates matching addresses from the specified geocoding request.
         * @param request The geocoding request to use.
         * @result The list of matching geocoding results, sorted by descending ranks.
         * @throws std::runtime_error If IO error occured during the calculation.
         */
        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const = 0;

    protected:
        /**
         * The default constructor.
         */
        GeocodingService();
    };
    
}

#endif

#endif
