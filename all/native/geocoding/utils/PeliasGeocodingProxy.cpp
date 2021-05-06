#ifdef _CARTO_GEOCODING_SUPPORT

#include "PeliasGeocodingProxy.h"
#include "core/Variant.h"
#include "components/Exceptions.h"
#include "geocoding/GeocodingAddress.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "geometry/Geometry.h"
#include "geometry/GeoJSONGeometryReader.h"
#include "projections/Projection.h"
#include "utils/Log.h"

#include <picojson/picojson.h>

namespace carto {

    std::vector<std::shared_ptr<GeocodingResult> > PeliasGeocodingProxy::ReadResponse(const std::string& responseString, const std::shared_ptr<Projection>& proj) {
        picojson::value response;
        std::string err = picojson::parse(response, responseString);
        if (!err.empty()) {
            throw GenericException("Failed to parse response", err);
        }

        if (!response.get("features").is<picojson::array>()) {
            throw GenericException("No features in the response");
        }

        GeoJSONGeometryReader reader;
        reader.setTargetProjection(proj);

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const picojson::value& featureInfo : response.get("features").get<picojson::array>()) {
            const picojson::value& properties = featureInfo.get("properties");

            auto extractAddressField = [&](const std::string& key) -> std::string {
                if (properties.get(key).is<std::string>()) {
                    return properties.get(key).get<std::string>();
                }
                return std::string();
            };

            std::string country       = extractAddressField("country");
            std::string region        = extractAddressField("region");
            std::string county        = extractAddressField("county");
            std::string locality      = extractAddressField("locality");
            std::string neighbourhood = extractAddressField("neighbourhood");
            std::string street        = extractAddressField("street");
            std::string postcode      = extractAddressField("postalcode");
            std::string houseNumber   = extractAddressField("housenumber");
            std::string name          = extractAddressField("name");
            if (name == houseNumber + " " + street) {
                name.clear();
            }

            GeocodingAddress address(country, region, county, locality, neighbourhood, street, postcode, houseNumber, name, std::vector<std::string>());
            float rank = static_cast<float>(properties.contains("confidence") ? properties.get("confidence").get<double>() : 0.5);

            std::shared_ptr<Geometry> geometry = reader.readGeometry(featureInfo.get("geometry").serialize());

            auto feature = std::make_shared<Feature>(geometry, Variant());
            auto featureCollection = std::make_shared<FeatureCollection>(std::vector<std::shared_ptr<Feature> > { feature });

            auto result = std::make_shared<GeocodingResult>(proj, address, rank, featureCollection);
            results.push_back(std::move(result));
        }
        return results;
    }

    PeliasGeocodingProxy::PeliasGeocodingProxy() {
    }
    
}

#endif
