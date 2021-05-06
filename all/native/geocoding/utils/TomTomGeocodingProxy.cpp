#ifdef _CARTO_GEOCODING_SUPPORT

#include "TomTomGeocodingProxy.h"
#include "core/Variant.h"
#include "components/Exceptions.h"
#include "geocoding/GeocodingAddress.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "geometry/PointGeometry.h"
#include "projections/Projection.h"
#include "utils/GeneralUtils.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>

#include <picojson/picojson.h>

namespace carto {

    std::vector<std::shared_ptr<GeocodingResult> > TomTomGeocodingProxy::ReadResponse(const std::string& responseString, const std::shared_ptr<Projection>& proj) {
        picojson::value response;
        std::string err = picojson::parse(response, responseString);
        if (!err.empty()) {
            throw GenericException("Failed to parse response", err);
        }

        const picojson::array* resultList = nullptr;
        if (response.get("results").is<picojson::array>()) {
            resultList = &response.get("results").get<picojson::array>();
        } else if (response.get("addresses").is<picojson::array>()) {
            resultList = &response.get("addresses").get<picojson::array>();
        } else {
            throw GenericException("No results in the response");
        }

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const picojson::value& resultInfo : *resultList) {
            const picojson::value& addressInfo = resultInfo.get("address");

            auto extractAddressField = [&](const std::string& key) -> std::string {
                if (addressInfo.get(key).is<std::string>()) {
                    std::vector<std::string> items = GeneralUtils::Split(addressInfo.get(key).get<std::string>(), ',');
                    if (!items.empty()) {
                        return items.front();
                    }
                }
                return std::string();
            };

            std::string country       = extractAddressField("country");
            std::string region        = extractAddressField("countrySubdivisionName");
            std::string county        = extractAddressField("countrySecondarySubdivision");
            std::string locality      = extractAddressField("municipality");
            std::string neighbourhood = extractAddressField("municipalitySubdivision");
            std::string street        = extractAddressField("streetName");
            std::string postcode      = extractAddressField("postalCode");
            std::string houseNumber   = extractAddressField("streetNumber");

            std::string name;
            std::vector<std::string> categories;
            if (resultInfo.get("poi").is<picojson::object>()) {
                const picojson::value& poiInfo = resultInfo.get("poi");

                name = poiInfo.contains("name") ? poiInfo.get("name").get<std::string>() : std::string();
                if (poiInfo.get("categories").is<picojson::array>()) {
                    for (const picojson::value& categoryInfo : poiInfo.get("categories").get<picojson::array>()) {
                        categories.push_back(categoryInfo.get<std::string>());
                    }
                }
            }

            GeocodingAddress address(country, region, county, locality, neighbourhood, street, postcode, houseNumber, name, categories);
            float rank = static_cast<float>(resultInfo.contains("score") ? resultInfo.get("score").get<double>() / 100.0 : 0.5);

            double lat = 0, lon = 0;
            const picojson::value& positionInfo = resultInfo.get("position");
            if (positionInfo.is<std::string>()) {
                std::vector<std::string> latLon = GeneralUtils::Split(positionInfo.get<std::string>(), ',');
                lat = boost::lexical_cast<double>(latLon.at(0));
                lon = boost::lexical_cast<double>(latLon.at(1));
            } else {
                lat = positionInfo.get("lat").is<double>() ? positionInfo.get("lat").get<double>() : boost::lexical_cast<double>(positionInfo.get("lat").get<std::string>());
                lon = positionInfo.get("lon").is<double>() ? positionInfo.get("lon").get<double>() : boost::lexical_cast<double>(positionInfo.get("lon").get<std::string>());
            }
            auto geometry = std::make_shared<PointGeometry>(proj->fromWgs84(MapPos(lon, lat)));

            auto feature = std::make_shared<Feature>(geometry, Variant());
            auto featureCollection = std::make_shared<FeatureCollection>(std::vector<std::shared_ptr<Feature> > { feature });

            auto result = std::make_shared<GeocodingResult>(proj, address, rank, featureCollection);
            results.push_back(std::move(result));
        }
        return results;
    }

    TomTomGeocodingProxy::TomTomGeocodingProxy() {
    }
    
}

#endif
