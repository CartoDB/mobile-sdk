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
        
    MapPos EPSG3857::fromWgs84(const MapPos& wgs84Pos) const {
        double num = wgs84Pos.getX() * Const::DEG_TO_RAD;
        double x = EARTH_RADIUS * num;
        double a = wgs84Pos.getY() * Const::DEG_TO_RAD;
        double y = HALF_EARTH_RADIUS * std::log((1 + std::sin(a)) / (1 - std::sin(a)));
        return MapPos(x, y, wgs84Pos.getZ());
    }
    
    MapPos EPSG3857::toWgs84(const MapPos& mapPos) const {
        double num3 = mapPos.getX() / EARTH_RADIUS;
        double num4 = num3 * Const::RAD_TO_DEG;
        double num5 = std::floor((double)((num4 + 180.0) / 360.0));
        double num6 = num4 - (num5 * 360.0);
        double num7 = 90 * Const::DEG_TO_RAD - (2 * std::atan(std::exp((-1 * mapPos.getY()) / EARTH_RADIUS)));
        return MapPos(num6, num7 * Const::RAD_TO_DEG, mapPos.getZ());
    }
    
    std::string EPSG3857::getName() const {
        return "EPSG:3857";
    }
        
    const double EPSG3857::HALF_EARTH_RADIUS = EPSG3857::EARTH_RADIUS / 2.0;
        
    const double EPSG3857::METERS_TO_INTERNAL_EQUATOR = Const::WORLD_SIZE / (2 * std::atan(1.0) * 4 * EPSG3857::EARTH_RADIUS);
    
}
