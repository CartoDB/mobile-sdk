#include "Address.h"

#include <sstream>

namespace carto {

    Address::Address() :
        _country(),
        _region(),
        _county(),
        _locality(),
        _neighbourhood(),
        _street(),
        _postcode(),
        _houseNumber(),
        _name(),
        _categories()
    {
    }

    Address::Address(const std::string& country, const std::string& region, const std::string& county, const std::string& locality, const std::string& neighbourhood, const std::string& street, const std::string& postcode, const std::string& houseNumber, const std::string& name, const std::vector<std::string>& categories) :
        _country(country),
        _region(region),
        _county(county),
        _locality(locality),
        _neighbourhood(neighbourhood),
        _street(street),
        _postcode(postcode),
        _houseNumber(houseNumber),
        _name(name),
        _categories(categories)
    {
    }

    Address::~Address() {
    }

    const std::string& Address::getCountry() const {
        return _country;
    }

    const std::string& Address::getRegion() const {
        return _region;
    }

    const std::string& Address::getCounty() const {
        return _county;
    }

    const std::string& Address::getLocality() const {
        return _locality;
    }

    const std::string& Address::getNeighbourhood() const {
        return _neighbourhood;
    }

    const std::string& Address::getStreet() const {
        return _street;
    }

    const std::string& Address::getPostcode() const {
        return _postcode;
    }

    const std::string& Address::getHouseNumber() const {
        return _houseNumber;
    }

    const std::string& Address::getName() const {
        return _name;
    }

    const std::vector<std::string>& Address::getCategories() const {
        return _categories;
    }

    bool Address::operator ==(const Address& address) const {
        return _country == address._country && _region == address._region && _county == address._county && _locality == address._locality && _neighbourhood == address._neighbourhood && _street == address._street && _postcode == address._postcode && _houseNumber == address._houseNumber && _name == address._name && _categories == address._categories;
    }
    
    bool Address::operator !=(const Address& address) const {
        return !(*this == address);
    }
        
    int Address::hash() const {
        return static_cast<int>(std::hash<std::string>()(toString()));
    }

    std::string Address::toString() const {
        std::vector<std::pair<std::string, std::string> > items {
            { "country",       _country       },
            { "region",        _region        },
            { "county",        _county        },
            { "locality",      _locality      },
            { "neighbourhood", _neighbourhood },
            { "street",        _street        },
            { "postcode",      _postcode      },
            { "houseNumber",   _houseNumber   },
            { "name",          _name          }
        };
        std::stringstream ss;
        ss << "Address [";
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
