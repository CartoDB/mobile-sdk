#include "TileUtils.h"
#include "projections/Projection.h"
#include "utils/Log.h"

#include <cmath>

namespace carto {

    MapTile TileUtils::CalculateMapTile(const MapPos& mapPos, int zoom, const std::shared_ptr<Projection>& proj) {
        double tileWidth = proj->getBounds().getDelta().getX() / (1 << zoom);
        double tileHeight = proj->getBounds().getDelta().getY() / (1 << zoom);
        MapVec mapVec = mapPos - proj->getBounds().getMin();
        int x = static_cast<int>(std::floor(mapVec.getX() / tileWidth));
        int y = static_cast<int>(std::floor(mapVec.getY() / tileHeight));
        return MapTile(x, y, zoom, 0);
    }

    MapTile TileUtils::CalculateClippedMapTile(const MapPos& mapPos, int zoom, const std::shared_ptr<Projection>& proj) {
        MapTile mapTile = CalculateMapTile(mapPos, zoom, proj);
        int maxExtent = (1 << mapTile.getZoom()) - 1;
        return MapTile(std::max(0, std::min(maxExtent, mapTile.getX())), std::max(0, std::min(maxExtent, mapTile.getY())), mapTile.getZoom(), mapTile.getFrameNr());
    }

    MapPos TileUtils::CalculateMapTileOrigin(const MapTile& mapTile, const std::shared_ptr<Projection>& proj) {
        double tileWidth = proj->getBounds().getDelta().getX() / (1 << mapTile.getZoom());
        double tileHeight = proj->getBounds().getDelta().getY() / (1 << mapTile.getZoom());
        MapVec mapVec(mapTile.getX() * tileWidth, mapTile.getY() * tileHeight);
        return proj->getBounds().getMin() + mapVec;
    }

    MapBounds TileUtils::CalculateMapTileBounds(const MapTile& mapTile, const std::shared_ptr<Projection>& proj) {
        MapPos pos0 = CalculateMapTileOrigin(mapTile, proj);
        MapPos pos1 = CalculateMapTileOrigin(MapTile(mapTile.getX() + 1, mapTile.getY() + 1, mapTile.getZoom(), mapTile.getFrameNr()), proj);
        return MapBounds(pos0, pos1);
    }
    
    TileUtils::TileUtils() {
    }

}
