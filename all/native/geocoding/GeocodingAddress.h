/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGADDRESS_H_
#define _CARTO_GEOCODINGADDRESS_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include "core/Address.h"

namespace carto {

    /**
     * A geographical address describing country, region, county and other information.
     */
    class GeocodingAddress : public Address {
    public:
        /**
         * Constructs an empty Address object.
         */
        GeocodingAddress();
        /**
         * Constructs a new address from the the country, region, county and other attributes.
         * @param country The country name of the address.
         * @param region The region name of the address.
         * @param county The county name of the address.
         * @param locality The locality name of the address.
         * @param neighbourhood The neighbourhood name of the address.
         * @param street The street name of the address.
         * @param postcode The postcode of the address.
         * @param houseNumber The house number of the address.
         * @param name The name of the address.
         * @param categories The category tags describing the address.
         */
        GeocodingAddress(const std::string& country, const std::string& region, const std::string& county, const std::string& locality, const std::string& neighbourhood, const std::string& street, const std::string& postcode, const std::string& houseNumber, const std::string& name, const std::vector<std::string>& categories);
        virtual ~GeocodingAddress();
    };
    
}

#endif

#endif
