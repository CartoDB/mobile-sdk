#include "TorqueTileDecoder.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "styles/CartoCSSStyleSet.h"
#include "vectortiles/utils/MapnikVTLogger.h"
#include "vectortiles/utils/VTBitmapLoader.h"
#include "vectortiles/utils/CartoCSSAssetLoader.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <vt/Tile.h>
#include <mapnikvt/Value.h>
#include <mapnikvt/SymbolizerContext.h>
#include <mapnikvt/TorqueFeatureDecoder.h>
#include <mapnikvt/TorqueTileReader.h>
#include <cartocss/TorqueCartoCSSMapLoader.h>

#include <boost/lexical_cast.hpp>

namespace carto {
    
    TorqueTileDecoder::TorqueTileDecoder(const std::shared_ptr<CartoCSSStyleSet>& styleSet) :
        _resolution(256),
        _map(),
        _symbolizerContext(),
        _styleSet(),
        _mutex()
    {
        if (!styleSet) {
            throw NullArgumentException("Null styleSet");
        }

        updateCurrentStyle(styleSet);
    }
    
    TorqueTileDecoder::~TorqueTileDecoder() {
    }

    int TorqueTileDecoder::getFrameCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::static_pointer_cast<mvt::TorqueMap>(_map)->getTorqueSettings().frameCount;
    }

    std::shared_ptr<CartoCSSStyleSet> TorqueTileDecoder::getStyleSet() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _styleSet;
    }
    
    void TorqueTileDecoder::setStyleSet(const std::shared_ptr<CartoCSSStyleSet>& styleSet) {
        if (!styleSet) {
            throw NullArgumentException("Null styleSet");
        }

        updateCurrentStyle(styleSet);
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
            Log::Warn("TorqueTileDecoder::decodeTile: Null tile data");
            return std::shared_ptr<TileMap>();
        }
        if (tileData->empty()) {
            return std::shared_ptr<TileMap>();
        }

        int resolution;
        std::shared_ptr<mvt::TorqueMap> map;
        std::shared_ptr<mvt::SymbolizerContext> symbolizerContext;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            resolution = _resolution;
            map = _map;
            symbolizerContext = _symbolizerContext;
        }
    
        try {
            auto logger = std::make_shared<MapnikVTLogger>("TorqueTileDecoder");
            mvt::TorqueFeatureDecoder decoder(*tileData->getDataPtr(), resolution, logger);
            decoder.setTransform(calculateTileTransform(tile, targetTile));

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
        }
        return std::shared_ptr<TileMap>();
    }

    void TorqueTileDecoder::updateCurrentStyle(const std::shared_ptr<CartoCSSStyleSet>& styleSet) {
        std::lock_guard<std::mutex> lock(_mutex);

        std::shared_ptr<mvt::TorqueMap> map;
        try {
            auto logger = std::make_shared<MapnikVTLogger>("TorqueTileDecoder");
            auto assetLoader = std::make_shared<CartoCSSAssetLoader>("", std::shared_ptr<AssetPackage>());
            css::TorqueCartoCSSMapLoader mapLoader(assetLoader, logger);
            mapLoader.setIgnoreLayerPredicates(true);
            map = mapLoader.loadMap(styleSet->getCartoCSS());
        }
        catch (const std::exception& ex) {
            throw ParseException("Style parsing failed", ex.what());
        }

        auto bitmapLoader = std::make_shared<VTBitmapLoader>("", std::shared_ptr<AssetPackage>());
        auto fontManager = std::make_shared<vt::FontManager>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
        auto bitmapManager = std::make_shared<vt::BitmapManager>(bitmapLoader);
        auto strokeMap = std::make_shared<vt::StrokeMap>(1);
        auto glyphMap = std::make_shared<vt::GlyphMap>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
        mvt::SymbolizerContext::Settings settings(DEFAULT_TILE_SIZE, std::map<std::string, mvt::Value>());
        auto symbolizerContext = std::make_shared<mvt::SymbolizerContext>(bitmapManager, fontManager, strokeMap, glyphMap, settings);

        _map = map;
        _symbolizerContext = symbolizerContext;
        _styleSet = styleSet;
    }
    
    const int TorqueTileDecoder::DEFAULT_TILE_SIZE = 256;
    const int TorqueTileDecoder::GLYPHMAP_SIZE = 2048;
}
