#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingAddress.h"

namespace carto {

    GeocodingAddress::GeocodingAddress(const std::string& country, const std::string& region, const std::string& county, const std::string& locality, const std::string& neighbourhood, const std::string& street, const std::string& houseNumber, const std::string& name) :
        _country(country),
        _region(region),
        _county(county),
        _locality(locality),
        _neighbourhood(neighbourhood),
        _street(street),
        _houseNumber(houseNumber),
        _name(name)
    {
    }

    GeocodingAddress::~GeocodingAddress() {
    }

    const std::string& GeocodingAddress::getCountry() const {
        return _country;
    }

    const std::string& GeocodingAddress::getRegion() const {
        return _region;
    }

    const std::string& GeocodingAddress::getCounty() const {
        return _county;
    }

    const std::string& GeocodingAddress::getLocality() const {
        return _locality;
    }

    const std::string& GeocodingAddress::getNeighbourhood() const {
        return _neighbourhood;
    }

    const std::string& GeocodingAddress::getStreet() const {
        return _street;
    }

    const std::string& GeocodingAddress::getHouseNumber() const {
        return _houseNumber;
    }

    const std::string& GeocodingAddress::getName() const {
        return _name;
    }
    std::string GeocodingAddress::toString() const {
        return "GeocodingAddress [name='" + _name + "']";
    }
    
}

#endif
