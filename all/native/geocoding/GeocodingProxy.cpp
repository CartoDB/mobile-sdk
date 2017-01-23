#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingProxy.h"
#include "core/Variant.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "projections/Projection.h"

#include <geocoding/Geocoder.h>
#include <geocoding/RevGeocoder.h>

#include <functional>

namespace {

    struct ValueConverter : boost::static_visitor<carto::Variant> {
        carto::Variant operator() (boost::blank) const { return carto::Variant(); }
        template <typename T> carto::Variant operator() (T val) const { return carto::Variant(val); }
    };

}

namespace carto {

    std::vector<std::shared_ptr<GeocodingResult> > GeocodingProxy::CalculateAddresses(const std::shared_ptr<geocoding::Geocoder>& geocoder, const std::shared_ptr<GeocodingRequest>& request) {
        std::vector<std::pair<geocoding::Address, float> > addrs = geocoder->findAddresses(request->getQuery());

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const std::pair<geocoding::Address, float>& addr : addrs) {
            results.push_back(TranslateAddress(request->getProjection(), addr.first, addr.second));
        }
        return results;
    }

    std::vector<std::shared_ptr<GeocodingResult> > GeocodingProxy::CalculateAddresses(const std::shared_ptr<geocoding::RevGeocoder>& revGeocoder, const std::shared_ptr<ReverseGeocodingRequest>& request) {
        MapPos pos = request->getProjection()->toWgs84(request->getPoint());
        std::vector<std::pair<geocoding::Address, float> > addrs = revGeocoder->findAddresses(pos.getX(), pos.getY());

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const std::pair<geocoding::Address, float>& addr : addrs) {
            results.push_back(TranslateAddress(request->getProjection(), addr.first, addr.second));
        }
        return results;
    }

    std::shared_ptr<GeocodingResult> GeocodingProxy::TranslateAddress(const std::shared_ptr<Projection>& proj, const geocoding::Address& addr, float rank) {
        std::vector<std::shared_ptr<Feature> > features;
        std::transform(addr.features.begin(), addr.features.end(), std::back_inserter(features), std::bind(&GeocodingProxy::TranslateFeature, proj, std::placeholders::_1));
        auto featureCollection = std::make_shared<FeatureCollection>(features);
        std::vector<std::string> categories(addr.categories.begin(), addr.categories.end());
        Address address(addr.country, addr.region, addr.county, addr.locality, addr.neighbourhood, addr.street, addr.houseNumber, addr.name, categories);
        return std::make_shared<GeocodingResult>(proj, address, rank, featureCollection);
    }

    std::shared_ptr<Feature> GeocodingProxy::TranslateFeature(const std::shared_ptr<Projection>& proj, const geocoding::Feature& feature) {
        std::map<std::string, Variant> properties;
        if (feature.getId()) {
            properties["_id"] = Variant(static_cast<long long>(feature.getId()));
        }
        for (auto it = feature.getProperties().begin(); it != feature.getProperties().end(); it++) {
            properties[it->first] = boost::apply_visitor(ValueConverter(), it->second);
        }
        std::shared_ptr<Geometry> geom = TranslateGeometry(proj, feature.getGeometry());
        return std::make_shared<Feature>(geom, Variant(properties));
    }

    std::shared_ptr<Geometry> GeocodingProxy::TranslateGeometry(const std::shared_ptr<Projection>& proj, const std::shared_ptr<geocoding::Geometry>& geom) {
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
        }
        else if (auto lineGeom = std::dynamic_pointer_cast<geocoding::LineGeometry>(geom)) {
            return std::make_shared<LineGeometry>(translateRing(lineGeom->getPoints()));
        }
        else if (auto polygonGeom = std::dynamic_pointer_cast<geocoding::PolygonGeometry>(geom)) {
            std::vector<std::vector<MapPos> > holes;
            std::transform(polygonGeom->getHoles().begin(), polygonGeom->getHoles().end(), std::back_inserter(holes), translateRing);
            return std::make_shared<PolygonGeometry>(translateRing(polygonGeom->getPoints()), holes);
        }
        else if (auto multiGeom = std::dynamic_pointer_cast<geocoding::MultiGeometry>(geom)) {
            std::vector<std::shared_ptr<Geometry> > geometries;
            std::transform(multiGeom->getGeometries().begin(), multiGeom->getGeometries().end(), std::back_inserter(geometries), std::bind(&GeocodingProxy::TranslateGeometry, proj, std::placeholders::_1));
            return std::make_shared<MultiGeometry>(geometries);
        }
        return std::shared_ptr<Geometry>();
    }
    
}

#endif
