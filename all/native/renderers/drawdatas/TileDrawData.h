/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TILEDRAWDATA_H_
#define _CARTO_TILEDRAWDATA_H_

#include <memory>

#include <vt/TileId.h>

namespace carto {
    namespace vt {
        class Tile;
    }

    class TileDrawData {
    public:
        TileDrawData(const vt::TileId& vtTileId, const std::shared_ptr<const vt::Tile>& vtTile, long long tileId, bool preloadingTile);
        virtual ~TileDrawData();

        const vt::TileId& getVTTileId() const;
        const std::shared_ptr<const vt::Tile>& getVTTile() const;
        
        bool isPreloadingTile() const;
        long long getTileId() const;
    
    private:
        vt::TileId _vtTileId;
        std::shared_ptr<const vt::Tile> _vtTile;

        long long _tileId;
        bool _preloadingTile;
    };
    
}

#endif
