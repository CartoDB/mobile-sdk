#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingProxy.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "projections/Projection.h"

#include <geocoding/Geocoder.h>
#include <geocoding/RevGeocoder.h>

namespace carto {

    std::vector<std::shared_ptr<GeocodingResult> > GeocodingProxy::CalculateAddresses(const std::shared_ptr<geocoding::Geocoder>& geocoder, const std::shared_ptr<Projection>& proj, const std::shared_ptr<GeocodingRequest>& request) {
        std::vector<geocoding::Address> addrs = geocoder->findAddresses(request->getQuery());

        std::vector<std::shared_ptr<GeocodingResult> > results;
        for (const geocoding::Address& addr : addrs) {
            results.push_back(TranslateAddress(proj, addr));
        }
        return results;
    }

    std::vector<std::shared_ptr<GeocodingResult> > GeocodingProxy::CalculateAddresses(const std::shared_ptr<geocoding::RevGeocoder>& revGeocoder, const std::shared_ptr<Projection>& proj, const std::shared_ptr<ReverseGeocodingRequest>& request) {
        MapPos pos = request->getProjection()->toWgs84(request->getPoint());
        boost::optional<geocoding::Address> addr = revGeocoder->findAddress(pos.getX(), pos.getY());

        std::vector<std::shared_ptr<GeocodingResult> > results;
        if (addr) {
            results.push_back(TranslateAddress(proj, *addr));
        }
        return results;
    }

    std::shared_ptr<GeocodingResult> GeocodingProxy::TranslateAddress(const std::shared_ptr<Projection>& proj, const geocoding::Address& addr) {
        std::shared_ptr<Geometry> geometry = TranslateGeometry(proj, addr.geometry);
        GeocodingAddress geoaddr(addr.country, addr.region, addr.county, addr.locality, addr.neighbourhood, addr.street, addr.houseNumber, addr.name);
        float rank = 1.0f; // TODO: implement
        return std::make_shared<GeocodingResult>(proj, addr.id, geoaddr, rank, geometry);
    }

    std::shared_ptr<Geometry> GeocodingProxy::TranslateGeometry(const std::shared_ptr<Projection>& proj, const std::shared_ptr<geocoding::Geometry>& geom) {
        auto translatePoint = [&proj](const cglib::vec2<double>& point) -> MapPos {
            return proj->fromWgs84(MapPos(point(1), point(0)));
        };
        auto translateRing = [&translatePoint](const std::vector<cglib::vec2<double> >& points) -> std::vector<MapPos> {
            std::vector<MapPos> poses;
            poses.reserve(points.size());
            for (const cglib::vec2<double>& point : points) {
                poses.push_back(translatePoint(point));
            }
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
            for (const std::vector<cglib::vec2<double>>& points : polygonGeom->getHoles()) {
                holes.push_back(translateRing(points));
            }
            return std::make_shared<PolygonGeometry>(translateRing(polygonGeom->getPoints()), holes);
        }
        else if (auto multiGeom = std::dynamic_pointer_cast<geocoding::MultiGeometry>(geom)) {
            std::vector<std::shared_ptr<Geometry> > geometries;
            for (const std::shared_ptr<geocoding::Geometry>& geom : multiGeom->getGeometries()) {
                geometries.push_back(TranslateGeometry(proj, geom));
            }
            return std::make_shared<MultiGeometry>(geometries);
        }
        return std::shared_ptr<Geometry>();
    }
    
}

#endif
