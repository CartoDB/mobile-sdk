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
         * Returns the autocomplete flag of the service.
         * @return The autocomplete flag of the service.
         */
        virtual bool isAutocomplete() const = 0;
        /**
         * Sets the autocomplete flag of the service.
         * By default this flag is off.
         * @param autocomplete The new value for autocomplete flag.
         */
        virtual void setAutocomplete(bool autocomplete) = 0;

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
         * Returns the maximum number of results the geocoding service returns.
         * @return The maximum number of results the geocoding service returns.
         */
        virtual int getMaxResults() const = 0;
        /**
         * Sets the maximum number of results the geocoding service returns.
         * The default number of results is service dependant (normally 10).
         * @param maxResults The new maximum number of results the geocoding service returns.
         */
        virtual void setMaxResults(int maxResults) = 0;

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
