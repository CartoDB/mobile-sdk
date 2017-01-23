/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGADDRESS_H_
#define _CARTO_GEOCODINGADDRESS_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include <string>
#include <vector>

namespace carto {
    class GeocodingAddress {
    public:
        GeocodingAddress(const std::string& country, const std::string& region, const std::string& county, const std::string& locality, const std::string& neighbourhood, const std::string& street, const std::string& houseNumber, const std::string& name, const std::vector<std::string>& categories);
        virtual ~GeocodingAddress();

        const std::string& getCountry() const;
        const std::string& getRegion() const;
        const std::string& getCounty() const;
        const std::string& getLocality() const;
        const std::string& getNeighbourhood() const;
        const std::string& getStreet() const;
        const std::string& getHouseNumber() const;
        const std::string& getName() const;

        const std::vector<std::string>& getCategories() const;

        std::string toString() const;
        
    private:
        std::string _country;
        std::string _region;
        std::string _county;
        std::string _locality;
        std::string _neighbourhood;
        std::string _street;
        std::string _houseNumber;
        std::string _name;
        std::vector<std::string> _categories;
    };
    
}

#endif

#endif
