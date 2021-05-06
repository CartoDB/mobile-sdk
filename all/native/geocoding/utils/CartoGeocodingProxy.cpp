#ifdef _CARTO_GEOCODING_SUPPORT

#include "CartoGeocodingProxy.h"
#include "core/Variant.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Const.h"

#include <geocoding/Geocoder.h>
#include <geocoding/RevGeocoder.h>

#include <cmath>
#include <functional>
#include <algorithm>

namespace {

    struct ValueConverter {
        carto::Variant operator() (std::monostate) const { return carto::Variant(); }
        template <typename T> carto::Variant operator() (T val) const { return carto::Variant(val); }
    };

}

namespace carto {

    std::vector<std::shared_ptr<GeocodingResult> > CartoGeocodingProxy::CalculateAddresses(const std::shared_ptr<geocoding::Geocoder>& geocoder, const std::shared_ptr<GeocodingRequest>& request) {
        geocoding::Geocoder::Options options;
        if (request->isLocationDefined()) {
            MapPos wgs84Center = request->getProjection()->toWgs84(request->getLocation());
            options.location = cglib::vec2<double>(wgs84Center.getX(), wgs84Center.getY());
        }
        if (request->getLocationRadius() > 0) {
            EPSG3857 epsg3857;
            MapPos wgs84Center = request->getProjection()->toWgs84(request->getLocation());
            double mercRadius = request->getLocationRadius() / std::cos(std::min(89.9, std::abs(wgs84Center.getY())) * Const::DEG_TO_RAD);
            MapPos mercPos0 = epsg3857.fromWgs84(wgs84Center) - MapVec(mercRadius, mercRadius);
            MapPos mercPos1 = epsg3857.fromWgs84(wgs84Center) + MapVec(mercRadius, mercRadius);
            mercPos0[0] = std::max(mercPos0[0], epsg3857.getBounds().getMin()[0] * 0.9999);
            mercPos1[0] = std::min(mercPos1[0], epsg3857.getBounds().getMax()[0] * 0.9999);
            MapPos wgs84Pos0 = epsg3857.toWgs84(mercPos0);
            MapPos wgs84Pos1 = epsg3857.toWgs84(mercPos1);
            options.bounds = cglib::bbox2<double>(cglib::vec2<double>(wgs84Pos0.getX(), wgs84Pos0.getY()), cglib::vec2<double>(wgs84Pos1.getX(), wgs84Pos1.getY()));
        }
        Variant locationSigma = request->getCustomParameter("ranking.location_sigma");
        if (locationSigma.getType() != VariantType::VARIANT_TYPE_NULL) {
            options.locationSigma = static_cast<float>(locationSigma.getDouble());
        }
        Variant matchRankWeight = request->getCustomParameter("ranking.match_weight");
        if (matchRankWeight.getType() != VariantType::VARIANT_TYPE_NULL) {
            options.matchRankWeight = static_cast<float>(matchRankWeight.getDouble());
        }
        Variant entityRankWeight = request->getCustomParameter("ranking.entity_weight");
        if (entityRankWeight.getType() != VariantType::VARIANT_TYPE_NULL) {
            options.entityRankWeight = static_cast<float>(entityRankWeight.getDouble());
        }
        Variant locationRankWeight = request->getCustomParameter("ranking.location_weight");
        if (locationRankWeight.getType() != VariantType::VARIANT_TYPE_NULL) {
            options.locationRankWeight = static_cast<float>(locationRankWeight.getDouble());
        }

        std::vector<std::pair<geocoding::Address, float> > addrs = geocoder->findAddresses(request->getQuery(), options);

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const std::pair<geocoding::Address, float>& addr : addrs) {
            results.push_back(TranslateAddress(request->getProjection(), addr.first, addr.second));
        }
        return results;
    }

    std::vector<std::shared_ptr<GeocodingResult> > CartoGeocodingProxy::CalculateAddresses(const std::shared_ptr<geocoding::RevGeocoder>& revGeocoder, const std::shared_ptr<ReverseGeocodingRequest>& request) {
        MapPos posWgs84 = request->getProjection()->toWgs84(request->getLocation());
        std::vector<std::pair<geocoding::Address, float> > addrs = revGeocoder->findAddresses(posWgs84.getX(), posWgs84.getY(), request->getSearchRadius());

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const std::pair<geocoding::Address, float>& addr : addrs) {
            results.push_back(TranslateAddress(request->getProjection(), addr.first, addr.second));
        }
        return results;
    }

    CartoGeocodingProxy::CartoGeocodingProxy() {
    }

    std::shared_ptr<GeocodingResult> CartoGeocodingProxy::TranslateAddress(const std::shared_ptr<Projection>& proj, const geocoding::Address& addr, float rank) {
        std::vector<std::shared_ptr<Feature> > features;
        std::transform(addr.features.begin(), addr.features.end(), std::back_inserter(features), std::bind(&CartoGeocodingProxy::TranslateFeature, proj, std::placeholders::_1));
        auto featureCollection = std::make_shared<FeatureCollection>(features);
        std::vector<std::string> categories(addr.categories.begin(), addr.categories.end());
        GeocodingAddress address(addr.country, addr.region, addr.county, addr.locality, addr.neighbourhood, addr.street, addr.postcode, addr.houseNumber, addr.name, categories);
        return std::make_shared<GeocodingResult>(proj, address, rank, featureCollection);
    }

    std::shared_ptr<Feature> CartoGeocodingProxy::TranslateFeature(const std::shared_ptr<Projection>& proj, const geocoding::Feature& feature) {
        std::map<std::string, Variant> properties;
        if (feature.getId()) {
            properties["_id"] = Variant(static_cast<long long>(feature.getId()));
        }
        for (auto it = feature.getProperties().begin(); it != feature.getProperties().end(); it++) {
            properties[it->first] = std::visit(ValueConverter(), it->second);
        }
        std::shared_ptr<Geometry> geom = TranslateGeometry(proj, feature.getGeometry());
        return std::make_shared<Feature>(geom, Variant(properties));
    }

    std::shared_ptr<Geometry> CartoGeocodingProxy::TranslateGeometry(const std::shared_ptr<Projection>& proj, const std::shared_ptr<geocoding::Geometry>& geom) {
        auto translatePoint = [&proj](const cglib::vec2<double>& point) -> MapPos {
            return proj->fromWgs84(MapPos(point(0), point(1)));
        };
        auto translateRing = [&translatePoint](const std::vector<cglib::vec2<double> >& points) -> std::vector<MapPos> {
            std::vector<MapPos> poses;
            poses.reserve(points.size());
            std::transform(points.begin(), points.end(), std::back_inserter(poses), translatePoint);
            return poses;
        };

        if (auto pointGeom = std::dynamic_pointer_cast<geocoding::PointGeometry>(geom)) {
            return std::make_shared<PointGeometry>(translatePoint(pointGeom->getPoint()));
        } else if (auto lineGeom = std::dynamic_pointer_cast<geocoding::LineGeometry>(geom)) {
            return std::make_shared<LineGeometry>(translateRing(lineGeom->getPoints()));
        } else if (auto polygonGeom = std::dynamic_pointer_cast<geocoding::PolygonGeometry>(geom)) {
            std::vector<std::vector<MapPos> > holes;
            std::transform(polygonGeom->getHoles().begin(), polygonGeom->getHoles().end(), std::back_inserter(holes), translateRing);
            return std::make_shared<PolygonGeometry>(translateRing(polygonGeom->getPoints()), holes);
        } else if (auto multiGeom = std::dynamic_pointer_cast<geocoding::MultiGeometry>(geom)) {
            std::vector<std::shared_ptr<Geometry> > geometries;
            std::transform(multiGeom->getGeometries().begin(), multiGeom->getGeometries().end(), std::back_inserter(geometries), std::bind(&CartoGeocodingProxy::TranslateGeometry, proj, std::placeholders::_1));
            return std::make_shared<MultiGeometry>(geometries);
        }
        return std::shared_ptr<Geometry>();
    }
    
}

#endif
