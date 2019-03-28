#include "EPSG3857.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cmath>

namespace carto {

    EPSG3857::EPSG3857() :
        Projection(MapBounds(MapPos(-Const::PI * EARTH_RADIUS, -Const::PI * EARTH_RADIUS), MapPos(Const::PI * EARTH_RADIUS, Const::PI * EARTH_RADIUS)))
    {
    }
    
    EPSG3857::~EPSG3857() {
    }
        
    double EPSG3857::fromInternalScale(double size) const {
        return size / METERS_TO_INTERNAL_EQUATOR;
    }
        
    double EPSG3857::toInternalScale(double meters) const {
        return meters * METERS_TO_INTERNAL_EQUATOR;
    }
        
    MapPos EPSG3857::fromInternal(const MapPos& mapPosInternal) const {
        return MapPos(mapPosInternal.getX() / METERS_TO_INTERNAL_EQUATOR, mapPosInternal.getY() / METERS_TO_INTERNAL_EQUATOR, mapPosInternal.getZ() / METERS_TO_INTERNAL_EQUATOR);
    }
    
    MapPos EPSG3857::toInternal(const MapPos& mapPos) const {
        return MapPos(mapPos.getX() * METERS_TO_INTERNAL_EQUATOR, mapPos.getY() * METERS_TO_INTERNAL_EQUATOR, mapPos.getZ() * METERS_TO_INTERNAL_EQUATOR);
    }

    MapPos EPSG3857::fromWgs84(const MapPos& wgs84Pos) const {
        double x = wgs84Pos.getX() * Const::DEG_TO_RAD * EARTH_RADIUS;
        double a = std::sin(wgs84Pos.getY() * Const::DEG_TO_RAD);
        double y = 0.5 * EARTH_RADIUS * std::log((1.0 + a) / (1.0 - a));
        return MapPos(x, y, wgs84Pos.getZ());
    }
    
    MapPos EPSG3857::toWgs84(const MapPos& mapPos) const {
        double x = mapPos.getX() / EARTH_RADIUS * Const::RAD_TO_DEG;
        double y = 90.0 - Const::RAD_TO_DEG * (2.0 * std::atan(std::exp(-mapPos.getY() / EARTH_RADIUS)));
        return MapPos(x, y, mapPos.getZ());
    }
    
    std::string EPSG3857::getName() const {
        return "EPSG:3857";
    }
        
    const double EPSG3857::METERS_TO_INTERNAL_EQUATOR = Const::WORLD_SIZE / (2.0 * Const::PI * EARTH_RADIUS);
    
}
