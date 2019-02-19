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
