#include "Projection.h"
#include "core/MapVec.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"

#include <cmath>

namespace carto {

    Projection::~Projection() {
    }
    
    MapBounds Projection::getBounds() const {
        return _bounds;
    }
        
    double Projection::getLocalScale(const MapPos& pos) const {
        return 1.0 / std::cos(toWgs84(pos).getY() * Const::DEG_TO_RAD);
    }
    
    double Projection::toInternalScale(const MapPos& pos, const MapVec& dir, double meters) const {
        MapPos wgs84Pos(toWgs84(pos));
    
        double coef = meters / EARTH_RADIUS;
        double sinCoef = std::sin(coef);
        double cosCoef = std::cos(coef);
        double sinY = std::sin(wgs84Pos.getY() * Const::DEG_TO_RAD);
        double cosY = std::cos(wgs84Pos.getY() * Const::DEG_TO_RAD);
        double lat = sinY * cosCoef + cosY * sinCoef * dir.getY();
        double lon = wgs84Pos.getX() + std::atan2(dir.getX() * sinCoef * cosY, cosCoef - sinY * lat) * Const::RAD_TO_DEG;
        MapPos internalPos(toInternal(pos));
        MapPos internalPos2(toInternal(fromWgs84(MapPos(lon, std::asin(lat) * Const::RAD_TO_DEG, pos.getZ()))));
        return GeomUtils::DistanceFromPoint(internalPos, internalPos2);
    }
        
    MapPos Projection::fromInternal(const MapPos& pos) const {
        const MapVec& boundsDelta = _bounds.getDelta();
        double offsetX = -_bounds.getMin().getX() - boundsDelta.getX() / 2;
        double offsetY = -_bounds.getMin().getY() - boundsDelta.getY() / 2;
        double scaleX = Const::WORLD_SIZE / boundsDelta.getX();
        double scaleY = Const::WORLD_SIZE / boundsDelta.getY();
        return MapPos(pos.getX() / scaleX - offsetX, pos.getY() / scaleY - offsetY, fromInternalScale(pos.getZ()));
    }
    
    MapPos Projection::toInternal(const MapPos& pos) const {
        const MapVec& boundsDelta = _bounds.getDelta();
        double offsetX = -_bounds.getMin().getX() - boundsDelta.getX() / 2;
        double offsetY = -_bounds.getMin().getY() - boundsDelta.getY() / 2;
        double scaleX = Const::WORLD_SIZE / boundsDelta.getX();
        double scaleY = Const::WORLD_SIZE / boundsDelta.getY();
        return MapPos((pos.getX() + offsetX) * scaleX, (pos.getY() + offsetY) * scaleY, toInternalScale(pos.getZ()));
    }

    MapPos Projection::fromLatLong(double lat, double lng) const {
        return fromWgs84(MapPos(lng, lat));
    }

    MapPos Projection::toLatLong(double x, double y) const {
        MapPos wgs84Pos = toWgs84(MapPos(x, y));
        return MapPos(wgs84Pos.getY(), wgs84Pos.getX());
    }
        
    Projection::Projection(const MapBounds& bounds) :
        _bounds(bounds)
    {
    }
    
}
