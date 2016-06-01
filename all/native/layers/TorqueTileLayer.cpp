#include "TorqueTileLayer.h"
#include "vectortiles/TorqueTileDecoder.h"

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

}
