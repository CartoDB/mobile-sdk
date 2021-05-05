#ifdef _CARTO_GEOCODING_SUPPORT

#include "MapBoxGeocodingProxy.h"
#include "core/Variant.h"
#include "geocoding/GeocodingAddress.h"
#include "components/Exceptions.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "geometry/Geometry.h"
#include "geometry/GeoJSONGeometryReader.h"
#include "projections/Projection.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#include <picojson/picojson.h>

namespace carto {

    std::vector<std::shared_ptr<GeocodingResult> > MapBoxGeocodingProxy::ReadResponse(const std::string& responseString, const std::shared_ptr<Projection>& proj) {
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
            std::string country, region, county, locality, neighbourhood, street, postcode, houseNumber, name;

            auto extractAddressField = [&](const picojson::value& info) {
                if (info.contains("id")) {
                    std::string id = info.get("id").to_str();
                    std::string type = id.substr(0, id.find("."));
                    std::string text = info.contains("text") ? info.get("text").to_str() : std::string();
                    if (type == "country") {
                        country = text;
                    } else if (type == "region") {
                        region = text;
                    } else if (type == "district") {
                        country = text;
                    } else if (type == "place" || type == "locality") {
                        locality = text;
                    } else if (type == "neighbourhood") {
                        neighbourhood = text;
                    } else if (type == "address") {
                        street = text;
                    } else if (type == "postcode") {
                        postcode = text;
                    } else {
                        name = text;
                    }
                }
            };

            extractAddressField(featureInfo);
            if (featureInfo.contains("context")) {
                for (const picojson::value& contextInfo : featureInfo.get("context").get<picojson::array>()) {
                    extractAddressField(contextInfo);
                }
            }
            
            if (featureInfo.contains("address")) {
                houseNumber = featureInfo.get("address").to_str();
            }

            std::vector<std::string> categories;
            if (featureInfo.contains("properties")) {
                const picojson::value& properties = featureInfo.get("properties");
                if (properties.contains("categories")) {
                    categories = GeneralUtils::Split(properties.get("categories").to_str(), ',');
                }
            }

            GeocodingAddress address(country, region, county, locality, neighbourhood, street, postcode, houseNumber, name, categories);
            float rank = static_cast<float>(featureInfo.contains("relevance") ? featureInfo.get("relevance").get<double>() : 0.5);

            std::shared_ptr<Geometry> geometry = reader.readGeometry(featureInfo.get("geometry").serialize());

            auto feature = std::make_shared<Feature>(geometry, Variant());
            auto featureCollection = std::make_shared<FeatureCollection>(std::vector<std::shared_ptr<Feature> > { feature });

            auto result = std::make_shared<GeocodingResult>(proj, address, rank, featureCollection);
            results.push_back(std::move(result));
        }
        return results;
    }

    MapBoxGeocodingProxy::MapBoxGeocodingProxy() {
    }
    
}

#endif
