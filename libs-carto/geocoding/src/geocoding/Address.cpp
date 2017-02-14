#include "Address.h"
#include "FeatureReader.h"
#include "StringUtils.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sqlite3pp.h>

namespace carto { namespace geocoding {
    std::string Address::buildTypeFilter(const std::vector<Type>& enabledTypes) {
        auto isEnabled = [&enabledTypes](Type type) {
            return std::find(enabledTypes.begin(), enabledTypes.end(), type) != enabledTypes.end();
        };

        std::vector<std::string> sqlFilters;
        if (isEnabled(Address::Type::POI)) {
            sqlFilters.push_back("name_id IS NOT NULL");
        }
        if (isEnabled(Address::Type::ADDRESS)) {
            sqlFilters.push_back("housenumbers IS NOT NULL AND name_id IS NULL");
        }
        if (isEnabled(Address::Type::STREET)) {
            sqlFilters.push_back("street_id IS NOT NULL AND housenumbers IS NULL AND name_id IS NULL");
        }
        if (isEnabled(Address::Type::NEIGHBOURHOOD)) {
            sqlFilters.push_back("neighbourhood_id IS NOT NULL AND street_id IS NULL AND housenumbers IS NULL AND name_id IS NULL");
        }
        if (isEnabled(Address::Type::LOCALITY)) {
            sqlFilters.push_back("locality_id IS NOT NULL AND neighbourhood_id IS NULL AND street_id IS NULL AND housenumbers IS NULL AND name_id IS NULL");
        }
        if (isEnabled(Address::Type::COUNTY)) {
            sqlFilters.push_back("county_id IS NOT NULL AND locality_id IS NULL AND neighbourhood_id IS NULL AND street_id IS NULL AND housenumbers IS NULL AND name_id IS NULL");
        }
        if (isEnabled(Address::Type::REGION)) {
            sqlFilters.push_back("region_id IS NOT NULL AND county_id IS NULL AND locality_id IS NULL AND neighbourhood_id IS NULL AND street_id IS NULL AND housenumbers IS NULL AND name_id IS NULL");
        }
        if (isEnabled(Address::Type::COUNTRY)) {
            sqlFilters.push_back("country_id IS NOT NULL AND region_id IS NULL AND county_id IS NULL AND locality_id IS NULL AND neighbourhood_id IS NULL AND street_id IS NULL AND housenumbers IS NULL AND name_id IS NULL");
        }

        std::string sqlFilter;
        for (std::size_t i = 0; i < sqlFilters.size(); i++) {
            sqlFilter += (i > 0 ? " OR (" : "(") + sqlFilters[i] + ")";
        }
        return sqlFilter;
    }
    
    bool Address::loadFromDB(sqlite3pp::database& db, std::uint64_t encodedId, const std::string& language, const PointConverter& converter) {
        auto loadName = [&db, &language](std::uint64_t id) -> std::string {
            if (id == 0) {
                return std::string();
            }
            
            sqlite3pp::query query(db, "SELECT name, lang FROM names WHERE id=:id");
            query.bind(":id", id);

            std::string defaultValue;
            for (auto qit = query.begin(); qit != query.end(); qit++) {
                auto value = qit->get<const char*>(0);
                auto lang = qit->get<const char*>(1);
                if (!lang) {
                    defaultValue = value;
                }
                else if (lang == language) {
                    return value;
                }
            }
            return defaultValue;
        };

        unsigned int entityId = static_cast<unsigned int>(encodedId & 0xffffffffU);
        unsigned int elementIndex = static_cast<unsigned int>(encodedId >> 32);
        
        sqlite3pp::query query(db, "SELECT country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id, name_id, features, housenumbers FROM entities WHERE id=:id");
        query.bind(":id", entityId);

        for (auto qit = query.begin(); qit != query.end(); qit++) {
            country       = loadName(qit->get<std::uint64_t>(0));
            region        = loadName(qit->get<std::uint64_t>(1));
            county        = loadName(qit->get<std::uint64_t>(2));
            locality      = loadName(qit->get<std::uint64_t>(3));
            neighbourhood = loadName(qit->get<std::uint64_t>(4));
            street        = loadName(qit->get<std::uint64_t>(5));
            postcode      = loadName(qit->get<std::uint64_t>(6));
            name          = loadName(qit->get<std::uint64_t>(7));

            // Load features
            features.clear();
            if (auto encodedFeatures = qit->get<const void *>(8)) {
                EncodingStream stream(encodedFeatures, qit->column_bytes(8));
                FeatureReader reader(stream, converter);
                if (elementIndex) {
                    for (unsigned int i = 1; i < elementIndex; i++) {
                        reader.readFeatureCollection();
                    }
                    std::vector<Feature> featureCollection = reader.readFeatureCollection();
                    features.insert(features.end(), featureCollection.begin(), featureCollection.end());
                }
                else {
                    while (!stream.eof()) {
                        std::vector<Feature> featureCollection = reader.readFeatureCollection();
                        features.insert(features.end(), featureCollection.begin(), featureCollection.end());
                    }
                }
            }

            // Decode house number
            houseNumber.clear();
            if (auto houseNumbers = qit->get<const char*>(9)) {
                std::vector<std::string> houseNumberVector;
                boost::split(houseNumberVector, houseNumbers, boost::is_any_of("|"), boost::token_compress_off);
                if (elementIndex) {
                    houseNumber = houseNumberVector.at(elementIndex - 1);
                }
                else {
                    houseNumber = (houseNumberVector.size() == 1 ? houseNumberVector.front() : std::string());
                }
            }

            // Load categories    
            categories.clear();
            sqlite3pp::query query2(db, "SELECT c.category FROM entitycategories ec, categories c WHERE ec.entity_id=:id AND ec.category_id=c.id");
            query2.bind(":id", entityId);
            for (auto qit2 = query2.begin(); qit2 != query2.end(); qit2++) {
                categories.insert(qit2->get<const char*>(0));
            }

            // Detect type
            type = Type::NONE;
            if (!name.empty()) {
                type = Type::POI;
            }
            else if (!houseNumber.empty()) {
                type = Type::ADDRESS;
            }
            else if (!street.empty()) {
                type = Type::STREET;
            }
            else if (!neighbourhood.empty()) {
                type = Type::NEIGHBOURHOOD;
            }
            else if (!locality.empty()) {
                type = Type::LOCALITY;
            }
            else if (!county.empty()) {
                type = Type::COUNTY;
            }
            else if (!region.empty()) {
                type = Type::REGION;
            }
            else if (!country.empty()) {
                type = Type::COUNTRY;
            }
            return true;
        }
        return false;
    }

    bool Address::merge(const Address& address) {
        if (address.type == type && address.country == country && address.region == region && address.county == county && address.locality == locality && address.neighbourhood == neighbourhood && address.street == street && address.name == name && address.houseNumber.empty() == houseNumber.empty()) {
            // Merge house numbers
            if (!houseNumber.empty() && ("," + houseNumber + ",").find(address.houseNumber) == std::string::npos) {
                houseNumber += "," + address.houseNumber;
            }

            // Merge features and categories
            features.insert(features.end(), address.features.begin(), address.features.end());
            categories.insert(address.categories.begin(), address.categories.end());
            return true;
        }
        return false;
    }

    std::string Address::toString() const {
        std::string str;
        if (!name.empty()) {
            str += name;
        }
        if (!houseNumber.empty()) {
            str += (str.empty() ? "" : ", ") + houseNumber;
        }
        if (!street.empty()) {
            str += (str.empty() ? "" : (houseNumber.empty() ? ", " : " ")) + street;
        }
        if (!neighbourhood.empty()) {
            str += (str.empty() ? "" : ", ") + neighbourhood;
        }
        if (!locality.empty()) {
            str += (str.empty() ? "" : ", ") + locality;
        }
        if (!county.empty()) {
            str += (str.empty() ? "" : ", ") + county;
        }
        if (!region.empty()) {
            str += (str.empty() ? "" : ", ") + region;
        }
        if (!country.empty()) {
            str += (str.empty() ? "" : ", ") + country;
        }
        if (!postcode.empty()) {
            str += (str.empty() ? "" : ", ") + postcode;
        }
        return str;
    }
} }
