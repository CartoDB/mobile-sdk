#include "MapTile.h"
#include "utils/Const.h"
#include "utils/GeneralUtils.h"

#include <cmath>
#include <sstream>
#include <functional>

namespace carto {

    MapTile::MapTile(int x, int y, int zoom, int frameNr) :
        _x(x),
        _y(y),
        _zoom(zoom),
        _frameNr(frameNr),
        _id((1 - GeneralUtils::IntPow(4, zoom)) / (1 - 4) + GeneralUtils::IntPow(2, zoom) * y + x + frameNr * TILE_ID_OFFSET)
    {
    }
        
    bool MapTile::operator ==(const MapTile& tile) const {
        return _id == tile._id && _x == tile._x && _y == tile._y && _zoom == tile._zoom && _frameNr == tile._frameNr;
    }
    
    bool MapTile::operator !=(const MapTile& tile) const {
        return !(operator==(tile));
    }
    
    int MapTile::getX() const {
        return _x;
    }
    
    int MapTile::getY() const {
        return _y;
    }
    
    int MapTile::getZoom() const {
        return _zoom;
    }
    
    int MapTile::getFrameNr() const {
        return _frameNr;
    }

    long long MapTile::getTileId() const {
        return _id;
    }

    MapTile MapTile::getParent() const {
        if (_zoom <= 0) {
            return *this;
        }
        return MapTile(_x / 2, _y / 2, _zoom - 1, _frameNr);
    }
    
    MapTile MapTile::getChild(int index) const {
        return MapTile(_x * 2 + (index % 2), _y * 2 + (index / 2), _zoom + 1, _frameNr);
    }

    MapTile MapTile::getFlipped() const {
        return MapTile(_x, (1 << _zoom) - 1 - _y, _zoom, _frameNr);
    }
    
    int MapTile::hash() const {
        return static_cast<int>(std::hash<long long>()(_id));
    }
    
    std::string MapTile::toString() const {
        std::stringstream ss;
        ss << "MapTile [x="<< _x << ", y=" << _y << ", zoom=" << _zoom << ", frameNr=" << _frameNr << ", id=" << _id << "]";
        return ss.str();
    }
    
    const long long MapTile::TILE_ID_OFFSET = (1 - GeneralUtils::IntPow(4, Const::MAX_SUPPORTED_ZOOM_LEVEL)) / (1 - 4);

}
