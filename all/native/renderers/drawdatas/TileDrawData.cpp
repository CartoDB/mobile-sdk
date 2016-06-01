#include "TileDrawData.h"
#include "vt/Tile.h"

namespace carto {

    TileDrawData::TileDrawData(const vt::TileId& vtTileId, const std::shared_ptr<const vt::Tile>& vtTile, long long tileId, bool preloadingTile) :
        _vtTileId(vtTileId),
        _vtTile(vtTile),
        _tileId(tileId),
        _preloadingTile(preloadingTile)
    {
    }
    
    TileDrawData::~TileDrawData() {
    }

    const vt::TileId& TileDrawData::getVTTileId() const {
         return _vtTileId;
    }
        
    const std::shared_ptr<const vt::Tile>& TileDrawData::getVTTile() const {
         return _vtTile;
    }
        
    bool TileDrawData::isPreloadingTile() const {
        return _preloadingTile;
    }
    
    long long TileDrawData::getTileId() const {
        return _tileId;
    }
    
}
