#include "RasterTileClickInfo.h"

namespace carto {

    RasterTileClickInfo::RasterTileClickInfo(const ClickInfo& clickInfo, const MapPos& clickPos, const MapTile& mapTile,
                                                   const Color& nearestColor, const Color& interpolatedColor,
                                                   const std::shared_ptr<Layer>& layer) :
        _clickInfo(clickInfo),
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
        return _clickInfo.getClickType();
    }
    
    const ClickInfo& RasterTileClickInfo::getClickInfo() const {
        return _clickInfo;
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
