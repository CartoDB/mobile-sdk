#include "RasterTileClickInfo.h"

namespace carto {

    RasterTileClickInfo::RasterTileClickInfo(ClickType::ClickType clickType, const MapPos& clickPos, const MapTile& mapTile,
                                                   const Color& nearestColor, const Color& interpolatedColor,
                                                   const std::shared_ptr<Layer>& layer) :
        _clickType(clickType),
        _clickPos(clickPos),
        _mapTile(mapTile),
        _nearestColor(nearestColor),
        _interpolatedColor(interpolatedColor),
        _layer(layer)
    {
    }
    
    RasterTileClickInfo::~RasterTileClickInfo() {
    }

    ClickType::ClickType RasterTileClickInfo::getClickType() const {
        return _clickType;
    }
    
    const MapPos& RasterTileClickInfo::getClickPos() const {
        return _clickPos;
    }
        
    const MapTile& RasterTileClickInfo::getMapTile() const {
        return _mapTile;
    }

    const Color& RasterTileClickInfo::getNearestColor() const {
        return _nearestColor;
    }
    
    const Color& RasterTileClickInfo::getInterpolatedColor() const {
        return _interpolatedColor;
    }
    
    std::shared_ptr<Layer> RasterTileClickInfo::getLayer() const {
        return _layer;
    }

}
