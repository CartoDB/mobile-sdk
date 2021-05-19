#include "GeocodingAddress.h"

namespace carto {

    GeocodingAddress::GeocodingAddress() :
        Address()
    {
    }

    GeocodingAddress::GeocodingAddress(const std::string& country, const std::string& region, const std::string& county, const std::string& locality, const std::string& neighbourhood, const std::string& street, const std::string& postcode, const std::string& houseNumber, const std::string& name, const std::vector<std::string>& categories) :
        Address(country, region, county, locality, neighbourhood, street, postcode, houseNumber, name, categories)
    {
    }

    GeocodingAddress::~GeocodingAddress() {
    }
    
}
