#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingAddress.h"

#include <sstream>

namespace carto {

    GeocodingAddress::GeocodingAddress(const std::string& country, const std::string& region, const std::string& county, const std::string& locality, const std::string& neighbourhood, const std::string& street, const std::string& houseNumber, const std::string& name, const std::vector<std::string>& categories) :
        _country(country),
        _region(region),
        _county(county),
        _locality(locality),
        _neighbourhood(neighbourhood),
        _street(street),
        _houseNumber(houseNumber),
        _name(name),
        _categories(categories)
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

    const std::vector<std::string>& GeocodingAddress::getCategories() const {
        return _categories;
    }

    std::string GeocodingAddress::toString() const {
        std::vector<std::pair<std::string, std::string> > items {
            { "country",       _country       },
            { "region",        _region        },
            { "county",        _county        },
            { "locality",      _locality      },
            { "neighbourhood", _neighbourhood },
            { "street",        _street        },
            { "houseNumber",   _houseNumber   },
            { "name",          _name          }
        };
        std::stringstream ss;
        ss << "GeocodingAddress [";
        bool empty = true;
        for (const std::pair<std::string, std::string>& item : items) {
            if (!item.second.empty()) {
                ss << (empty ? "" : ", ") << item.first << "=" << item.second;
                empty = false;
            }
        }
        if (!_categories.empty()) {
            ss << (empty ? "" : ", ") << "categories=";
            for (std::size_t i = 0; i < _categories.size(); i++) {
                ss << (i == 0 ? "" : ";") << _categories[i];
            }
        }
        ss << "]";
        return ss.str();
    }
    
}

#endif
