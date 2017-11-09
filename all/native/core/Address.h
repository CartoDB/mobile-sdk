/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ADDRESS_H_
#define _CARTO_ADDRESS_H_

#include <string>
#include <vector>

namespace carto {

    /**
     * A geographical address describing country, region, county and other information.
     */
    class Address {
    public:
        /**
         * Constructs an empty Address object.
         */
        Address();
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
        Address(const std::string& country, const std::string& region, const std::string& county, const std::string& locality, const std::string& neighbourhood, const std::string& street, const std::string& postcode, const std::string& houseNumber, const std::string& name, const std::vector<std::string>& categories);

        /**
         * Returns the country name included in the address.
         * @return The country name included in the address.
         */
        const std::string& getCountry() const;

        /**
         * Returns the region name included in the address.
         * @return The region name included in the address.
         */
        const std::string& getRegion() const;

        /**
         * Returns the county name included in the address.
         * @return The county name included in the address.
         */
        const std::string& getCounty() const;

        /**
         * Returns the locality (city, town, village) name included in the address.
         * @return The locality name included in the address.
         */
        const std::string& getLocality() const;

        /**
         * Returns the local neighbourhood name included in the address.
         * @return The neighbourhood name included in the address.
         */
        const std::string& getNeighbourhood() const;

        /**
         * Returns the street name included in the address.
         * @return The street name included in the address.
         */
        const std::string& getStreet() const;

        /**
         * Returns the postcode of the address.
         * @return The postcode of the address.
         */
        const std::string& getPostcode() const;

        /**
         * Returns the house number included in the address.
         * @return The house number included in the address.
         */
        const std::string& getHouseNumber() const;

        /**
         * Returns the name included in the address.
         * @return The name included in the address.
         */
        const std::string& getName() const;

        /**
         * Returns the list of category tags describing the address.
         * @return The list of category tags describing the address.
         */
        const std::vector<std::string>& getCategories() const;

        /**
         * Checks for equality between this and another address object.
         * @param address The other address object.
         * @return True if equal.
         */
        bool operator ==(const Address& address) const;
        /**
         * Checks for inequality between this and another address object.
         * @param address The other address object.
         * @return True if not equal.
         */
        bool operator !=(const Address& address) const;
        
        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;
        
        /**
         * Creates a string representation of this address object, useful for logging.
         * @return The string representation of this address object.
         */
        std::string toString() const;
        
    private:
        std::string _country;
        std::string _region;
        std::string _county;
        std::string _locality;
        std::string _neighbourhood;
        std::string _street;
        std::string _postcode;
        std::string _houseNumber;
        std::string _name;
        std::vector<std::string> _categories;
    };
    
}

#endif
