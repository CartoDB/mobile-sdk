#include "TorqueTileLayer.h"
#include "vectortiles/TorqueTileDecoder.h"

#include <vt/Tile.h>

namespace carto {

    TorqueTileLayer::TorqueTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<TorqueTileDecoder>& decoder) :
        VectorTileLayer(dataSource, decoder)
    {
        // Configure base class for Torque
        _useFBO = true;
        _useDepth = false;
        _useStencil = false;
        _useTileMapMode = true;
    }
    
    TorqueTileLayer::~TorqueTileLayer() {
    }

    int TorqueTileLayer::countVisibleFeatures(int frameNr) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        int count = 0;
        for (long long tileId : getVisibleTileIds()) {
            if (std::shared_ptr<VectorTileDecoder::TileMap> tileMap = getTileMap(tileId)) {
                auto it = tileMap->find(frameNr);
                if (it != tileMap->end()) {
                    count += it->second->getFeatureCount();
                }
            }
        }
        return count;
    }

}
