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
         * Returns the language of the expected results.
         * @return The language of the expected results. As ISO 639-1 code or empty string.
         */
        virtual std::string getLanguage() const = 0;
        /**
         * Sets the language of the expected results.
         * @param lang The language to use as ISO 639-1 code. Empty string can be used for default language.
         */
        virtual void setLanguage(const std::string& lang) = 0;

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
