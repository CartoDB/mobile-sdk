#ifdef _CARTO_GEOCODING_SUPPORT

#include "PeliasGeocodingProxy.h"
#include "core/Address.h"
#include "core/Variant.h"
#include "components/Exceptions.h"
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
            Log::Errorf("PeliasGeocodingProxy: Failed to parse response: %s", err.c_str());
            throw GenericException("Failed to parse response", err);
        }

        if (!response.get("features").is<picojson::array>()) {
            Log::Error("PeliasGeocodingProxy: No features in the response");
            throw GenericException("No features in the response");
        }

        GeoJSONGeometryReader reader;
        reader.setTargetProjection(proj);

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const picojson::value& featureInfo : response.get("features").get<picojson::array>()) {
             const picojson::value& properties = featureInfo.get("properties");

             std::string country       = properties.contains("country")       ? properties.get("country").get<std::string>() : std::string();
             std::string region        = properties.contains("region")        ? properties.get("region").get<std::string>() : std::string();
             std::string county        = properties.contains("county")        ? properties.get("county").get<std::string>() : std::string();
             std::string locality      = properties.contains("locality")      ? properties.get("locality").get<std::string>() : std::string();
             std::string neighbourhood = properties.contains("neighbourhood") ? properties.get("neighbourhood").get<std::string>() : std::string();
             std::string street        = properties.contains("street")        ? properties.get("street").get<std::string>() : std::string();
             std::string houseNumber   = properties.contains("housenumber")   ? properties.get("housenumber").get<std::string>() : std::string();
             std::string name          = properties.contains("name")          ? properties.get("name").get<std::string>() : std::string();
             if (name == houseNumber + " " + street) {
                 name.clear();
             }

             Address address(country, region, county, locality, neighbourhood, street, houseNumber, name, std::vector<std::string>());
             float rank = static_cast<float>(properties.contains("confidence") ? properties.get("confidence").get<double>() : 0.5);

             std::shared_ptr<Geometry> geometry = reader.readGeometry(featureInfo.get("geometry").serialize());

             auto feature = std::make_shared<Feature>(geometry, Variant());
             auto featureCollection = std::make_shared<FeatureCollection>(std::vector<std::shared_ptr<Feature> > { feature });

             auto result = std::make_shared<GeocodingResult>(proj, address, rank, featureCollection);
             results.push_back(result);
        }
        return results;
    }

    PeliasGeocodingProxy::PeliasGeocodingProxy() {
    }
    
}

#endif
