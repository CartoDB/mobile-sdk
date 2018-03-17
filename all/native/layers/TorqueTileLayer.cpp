#include "TorqueTileLayer.h"
#include "layers/VectorTileEventListener.h"
#include "renderers/MapRenderer.h"
#include "renderers/TileRenderer.h"
#include "vectortiles/TorqueTileDecoder.h"

#include <vt/Tile.h>

namespace carto {

    TorqueTileLayer::TorqueTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<TorqueTileDecoder>& decoder) :
        VectorTileLayer(dataSource, decoder)
    {
        // Configure base class for Torque
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

    bool TorqueTileLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState) {
        updateTileLoadListener();

        if (std::shared_ptr<MapRenderer> mapRenderer = _mapRenderer.lock()) {
            if (std::shared_ptr<TileRenderer> tileRenderer = getTileRenderer()) {
                float opacity = getOpacity();

                mapRenderer->clearAndBindScreenFBO(getTileDecoder()->getBackgroundColor(), false, false);

                tileRenderer->setInteractionMode(getVectorTileEventListener().get() ? true : false);
                tileRenderer->setSubTileBlending(false);
                bool refresh = tileRenderer->onDrawFrame(deltaSeconds, viewState);

                mapRenderer->blendAndUnbindScreenFBO(opacity);

                return refresh;
            }
        }
        return false;
    }
        
}
