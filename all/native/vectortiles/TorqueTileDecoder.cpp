#include "TorqueTileDecoder.h"
#include "core/BinaryData.h"
#include "vectortiles/CartoCSSStyleSet.h"
#include "vectortiles/utils/MapnikVTLogger.h"
#include "vectortiles/utils/VTBitmapLoader.h"
#include "vectortiles/utils/CartoCSSAssetLoader.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vt/Tile.h"
#include "Mapnikvt/Value.h"
#include "Mapnikvt/SymbolizerContext.h"
#include "Mapnikvt/TorqueFeatureDecoder.h"
#include "Mapnikvt/TorqueTileReader.h"
#include "cartocss/TorqueCartoCSSMapLoader.h"

#include <vector>

#include <boost/lexical_cast.hpp>

namespace carto {
    
    TorqueTileDecoder::TorqueTileDecoder(const std::shared_ptr<CartoCSSStyleSet>& styleSet) :
        _resolution(256), _map(), _symbolizerContext(), _styleSet(), _mutex()
    {
        setStyleSet(styleSet);
    }
    
    TorqueTileDecoder::~TorqueTileDecoder() {
    }

    int TorqueTileDecoder::getFrameCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_map) {
            return 0;
        }
        return std::static_pointer_cast<mvt::TorqueMap>(_map)->getTorqueSettings().frameCount;
    }

    std::shared_ptr<CartoCSSStyleSet> TorqueTileDecoder::getStyleSet() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _styleSet;
    }
    
    void TorqueTileDecoder::setStyleSet(const std::shared_ptr<CartoCSSStyleSet>& styleSet) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
    
            auto logger = std::make_shared<MapnikVTLogger>("TorqueTileDecoder");
            auto bitmapLoader = std::make_shared<VTBitmapLoader>("", std::shared_ptr<AssetPackage>());
            auto assetLoader = std::make_shared<CartoCSSAssetLoader>("", std::shared_ptr<AssetPackage>());
            mvt::SymbolizerContext::Settings settings(DEFAULT_TILE_SIZE, std::map<std::string, mvt::Value>());
            auto fontManager = std::make_shared<vt::FontManager>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
            auto bitmapManager = std::make_shared<vt::BitmapManager>(bitmapLoader);
            auto strokeMap = std::make_shared<vt::StrokeMap>(1);
            auto glyphMap = std::make_shared<vt::GlyphMap>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
            _symbolizerContext = std::make_shared<mvt::SymbolizerContext>(bitmapManager, fontManager, strokeMap, glyphMap, settings);
            try {
                css::TorqueCartoCSSMapLoader mapLoader(assetLoader, logger);
                mapLoader.setIgnoreLayerPredicates(true);
                _map = mapLoader.loadMap(styleSet->getCartoCSS());
            }
            catch (const std::exception& ex) {
                Log::Errorf("TorqueTileDecoder: Style parsing failed: %s", ex.what());
                return;
            }

            _styleSet = styleSet;
        }
        notifyDecoderChanged();
    }

    int TorqueTileDecoder::getResolution() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _resolution;
    }
    
    void TorqueTileDecoder::setResolution(int resolution) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _resolution = resolution;
        }
        notifyDecoderChanged();
    }
    
    Color TorqueTileDecoder::getBackgroundColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_map) {
            return Color(0);
        }
        return Color(std::static_pointer_cast<mvt::TorqueMap>(_map)->getTorqueSettings().clearColor.value());
    }
    
    std::shared_ptr<const vt::BitmapPattern> TorqueTileDecoder::getBackgroundPattern() const {
        return std::shared_ptr<const vt::BitmapPattern>();
    }
        
    int TorqueTileDecoder::getMinZoom() const {
        return 0;
    }
    
    int TorqueTileDecoder::getMaxZoom() const {
        return Const::MAX_SUPPORTED_ZOOM_LEVEL;
    }
        
    std::shared_ptr<TorqueTileDecoder::TileMap> TorqueTileDecoder::decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<BinaryData>& tileData) const {
        if (!tileData) {
            Log::Error("TorqueTileDecoder::decodeTile: Null tile data");
            return std::shared_ptr<TileMap>();
        }
        if (tileData->empty()) {
            return std::shared_ptr<TileMap>();
        }

        std::shared_ptr<mvt::TorqueMap> map;
        std::shared_ptr<mvt::SymbolizerContext> symbolizerContext;
        int resolution;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            map = _map;
            symbolizerContext = _symbolizerContext;
            resolution = _resolution;
        }
    
        if (!map || !symbolizerContext) {
            return std::shared_ptr<TileMap>();
        }
        try {
            auto logger = std::make_shared<MapnikVTLogger>("TorqueTileDecoder");
            mvt::TorqueFeatureDecoder decoder(*tileData->getDataPtr(), resolution, calculateTileTransform(tile, targetTile), logger);
            auto tileMap = std::make_shared<TileMap>();
            for (int frame = 0; frame < map->getTorqueSettings().frameCount; frame++) {
                mvt::TorqueTileReader reader(map, frame, true, *symbolizerContext, decoder);
                if (std::shared_ptr<vt::Tile> tile = reader.readTile(targetTile)) {
                    (*tileMap)[frame] = tile;
                }
            }
            return tileMap;
        } catch (const std::exception& ex) {
            Log::Errorf("TorqueTileDecoder::decodeTile: Exception while decoding: %s", ex.what());
            return std::shared_ptr<TileMap>();
        }
    }
    
    const int TorqueTileDecoder::DEFAULT_TILE_SIZE = 256;
    const int TorqueTileDecoder::GLYPHMAP_SIZE = 2048;
}
