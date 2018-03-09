#ifdef _CARTO_GEOCODING_SUPPORT

#include "TomTomGeocodingProxy.h"
#include "core/Address.h"
#include "core/Variant.h"
#include "components/Exceptions.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "geometry/PointGeometry.h"
#include "projections/Projection.h"
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

        if (!response.get("results").is<picojson::array>()) {
            throw GenericException("No results in the response");
        }

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const picojson::value& resultInfo : response.get("results").get<picojson::array>()) {
            const picojson::value& addressInfo = resultInfo.get("address");

            std::string country       = addressInfo.contains("country")       ? addressInfo.get("country").get<std::string>() : std::string();
            std::string region        = addressInfo.contains("countrySubdivisionName") ? addressInfo.get("countrySubdivisionName").get<std::string>() : std::string();
            std::string county        = addressInfo.contains("countrySecondarySubdivision") ? addressInfo.get("countrySecondarySubdivision").get<std::string>() : std::string();
            std::string locality      = addressInfo.contains("municipality")  ? addressInfo.get("municipality").get<std::string>() : std::string();
            std::string neighbourhood = addressInfo.contains("municipalitySubdivision") ? addressInfo.get("municipalitySubdivision").get<std::string>() : std::string();
            std::string street        = addressInfo.contains("streetName")    ? addressInfo.get("streetName").get<std::string>() : std::string();
            std::string postcode      = addressInfo.contains("postalCode")    ? addressInfo.get("postalCode").get<std::string>() : std::string();
            std::string houseNumber   = addressInfo.contains("streetNumber")  ? addressInfo.get("streetNumber").get<std::string>() : std::string();

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

            Address address(country, region, county, locality, neighbourhood, street, postcode, houseNumber, name, categories);
            float rank = static_cast<float>(resultInfo.contains("score") ? resultInfo.get("score").get<double>() / 100.0 : 0.5);

            const picojson::value& positionInfo = resultInfo.get("position");
            double lat = positionInfo.get("lat").is<double>() ? positionInfo.get("lat").get<double>() : boost::lexical_cast<double>(positionInfo.get("lat").get<std::string>());
            double lon = positionInfo.get("lon").is<double>() ? positionInfo.get("lon").get<double>() : boost::lexical_cast<double>(positionInfo.get("lon").get<std::string>());
            auto geometry = std::make_shared<PointGeometry>(proj->fromWgs84(MapPos(lon, lat)));

            auto feature = std::make_shared<Feature>(geometry, Variant());
            auto featureCollection = std::make_shared<FeatureCollection>(std::vector<std::shared_ptr<Feature> > { feature });

            auto result = std::make_shared<GeocodingResult>(proj, address, rank, featureCollection);
            results.push_back(result);
        }
        return results;
    }

    TomTomGeocodingProxy::TomTomGeocodingProxy() {
    }
    
}

#endif
