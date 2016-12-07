#include "CartoVectorTileDecoder.h"
#include "core/MapTile.h"
#include "core/MapBounds.h"
#include "core/BinaryData.h"
#include "core/Variant.h"
#include "components/Exceptions.h"
#include "geometry/Feature.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "graphics/Bitmap.h"
#include "styles/CompiledStyleSet.h"
#include "styles/CartoCSSStyleSet.h"
#include "vectortiles/utils/MapnikVTLogger.h"
#include "vectortiles/utils/GeometryConverter.h"
#include "vectortiles/utils/ValueConverter.h"
#include "vectortiles/utils/VTBitmapLoader.h"
#include "vectortiles/utils/CartoCSSAssetLoader.h"
#include "utils/AssetPackage.h"
#include "utils/FileUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <vt/Tile.h>
#include <mapnikvt/Value.h>
#include <mapnikvt/SymbolizerParser.h>
#include <mapnikvt/SymbolizerContext.h>
#include <mapnikvt/MBVTFeatureDecoder.h>
#include <mapnikvt/MBVTTileReader.h>
#include <mapnikvt/MapParser.h>
#include <cartocss/CartoCSSMapLoader.h>

#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace carto {
    
    CartoVectorTileDecoder::CartoVectorTileDecoder(const std::vector<std::string>& layerIds, const std::map<std::string, std::string>& layerStyles, const std::shared_ptr<AssetPackage>& assetPackage) :
        _logger(std::make_shared<MapnikVTLogger>("CartoVectorTileDecoder")),
        _assetPackage(assetPackage),
        _layerIds(layerIds),
        _layerStyles(),
        _layerMaps(),
        _backgroundColor(),
        _backgroundPattern(),
        _symbolizerContext()
    {
        mvt::SymbolizerContext::Settings settings(DEFAULT_TILE_SIZE, std::map<std::string, mvt::Value>());
        auto fontManager = std::make_shared<vt::FontManager>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
        auto bitmapLoader = std::make_shared<VTBitmapLoader>("", assetPackage);
        auto bitmapManager = std::make_shared<vt::BitmapManager>(bitmapLoader);
        auto strokeMap = std::make_shared<vt::StrokeMap>(STROKEMAP_SIZE);
        auto glyphMap = std::make_shared<vt::GlyphMap>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
        _symbolizerContext = std::make_shared<mvt::SymbolizerContext>(bitmapManager, fontManager, strokeMap, glyphMap, settings);

        if (assetPackage) {
            std::string fontPrefix = FileUtils::NormalizePath("fonts/");

            for (const std::string& assetName : assetPackage->getAssetNames()) {
                if (assetName.size() > fontPrefix.size() && assetName.substr(0, fontPrefix.size()) == fontPrefix) {
                    if (std::shared_ptr<BinaryData> fontData = assetPackage->loadAsset(assetName)) {
                        fontManager->loadFontData(*fontData->getDataPtr());
                    }
                }
            }
        }

        for (auto it = layerStyles.begin(); it != layerStyles.end(); it++) {
            updateLayerStyle(it->first, it->second);
        }
    }
    
    CartoVectorTileDecoder::~CartoVectorTileDecoder() {
    }

    std::vector<std::string> CartoVectorTileDecoder::getLayerIds() const {
        return _layerIds;
    }

    std::string CartoVectorTileDecoder::getLayerStyle(const std::string& layerId) const {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _layerStyles.find(layerId);
        if (it == _layerStyles.end()) {
            throw OutOfRangeException("Invalid layer id");
        }
        return it->second;
    }
    
    void CartoVectorTileDecoder::setLayerStyle(const std::string& layerId, const std::string& cartoCSS) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _layerStyles.find(layerId);
            if (it == _layerStyles.end()) {
                throw OutOfRangeException("Invalid layer id");
            }
            updateLayerStyle(layerId, cartoCSS);
        }
        notifyDecoderChanged();
    }

    Color CartoVectorTileDecoder::getBackgroundColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _backgroundColor;
    }
    
    std::shared_ptr<const vt::BitmapPattern> CartoVectorTileDecoder::getBackgroundPattern() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _backgroundPattern;
    }
        
    int CartoVectorTileDecoder::getMinZoom() const {
        return 0;
    }
    
    int CartoVectorTileDecoder::getMaxZoom() const {
        return Const::MAX_SUPPORTED_ZOOM_LEVEL;
    }

    std::shared_ptr<CartoVectorTileDecoder::TileFeature> CartoVectorTileDecoder::decodeFeature(long long id, const vt::TileId& tile, const std::shared_ptr<BinaryData>& tileData, const MapBounds& tileBounds) const {
        if (!tileData) {
            Log::Warn("CartoVectorTileDecoder::decodeFeature: Null tile data");
            return std::shared_ptr<TileFeature>();
        }
        if (tileData->empty()) {
            return std::shared_ptr<TileFeature>();
        }

        try {
            std::shared_ptr<mvt::MBVTFeatureDecoder> decoder;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_cachedFeatureDecoder.first != tileData) {
                    lock.unlock();
                    decoder = std::make_shared<mvt::MBVTFeatureDecoder>(*tileData->getDataPtr(), _logger);
                    lock.lock();
                    _cachedFeatureDecoder = std::make_pair(tileData, decoder);
                }
                else {
                    decoder = _cachedFeatureDecoder.second;
                }
            }

            std::string mvtLayerName;
            std::shared_ptr<mvt::Feature> mvtFeature = decoder->getFeature(id, mvtLayerName);
            if (!mvtFeature) {
                return std::shared_ptr<TileFeature>();
            }

            std::map<std::string, Variant> featureData;
            if (std::shared_ptr<const mvt::FeatureData> mvtFeatureData = mvtFeature->getFeatureData()) {
                for (const std::string& varName : mvtFeatureData->getVariableNames()) {
                    mvt::Value mvtValue;
                    mvtFeatureData->getVariable(varName, mvtValue);
                    featureData[varName] = boost::apply_visitor(ValueConverter(), mvtValue);
                }
            }

            auto convertFn = [&tileBounds](const cglib::vec2<float>& pos) {
                return MapPos(tileBounds.getMin().getX() + pos(0) * tileBounds.getDelta().getX(), tileBounds.getMax().getY() - pos(1) * tileBounds.getDelta().getY(), 0);
            };
            auto feature = std::make_shared<Feature>(convertGeometry(convertFn, mvtFeature->getGeometry()), Variant(featureData));
            return std::make_shared<TileFeature>(mvtFeature->getId(), mvtLayerName, feature);
        } catch (const std::exception& ex) {
            Log::Errorf("CartoVectorTileDecoder::decodeFeature: Exception while decoding: %s", ex.what());
        }
        return std::shared_ptr<TileFeature>();
    }
        
    std::shared_ptr<CartoVectorTileDecoder::TileMap> CartoVectorTileDecoder::decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<BinaryData>& tileData) const {
        if (!tileData) {
            Log::Warn("CartoVectorTileDecoder::decodeTile: Null tile data");
            return std::shared_ptr<TileMap>();
        }
        if (tileData->empty()) {
            return std::shared_ptr<TileMap>();
        }

        std::map<std::string, std::shared_ptr<mvt::Map> > layerMaps;
        std::shared_ptr<mvt::SymbolizerContext> symbolizerContext;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            layerMaps = _layerMaps;
            symbolizerContext = _symbolizerContext;
        }
    
        try {
            mvt::MBVTFeatureDecoder decoder(*tileData->getDataPtr(), _logger);
            decoder.setTransform(calculateTileTransform(tile, targetTile));
            decoder.setGlobalIdOverride(true, MapTile(tile.x, tile.y, tile.zoom, 0).getTileId());

            std::vector<std::shared_ptr<vt::Tile> > tiles(_layerIds.size());
            for (auto it = layerMaps.begin(); it != layerMaps.end(); it++) {
                mvt::MBVTTileReader reader(it->second, *symbolizerContext, decoder);
                reader.setLayerFilter(it->first);
                if (std::shared_ptr<vt::Tile> tile = reader.readTile(targetTile)) {
                    std::size_t index = std::distance(_layerIds.begin(), std::find(_layerIds.begin(), _layerIds.end(), it->first));
                    if (index < tiles.size()) {
                        tiles.push_back(tile);
                    }
                }
            }

            std::vector<std::shared_ptr<vt::TileLayer> > tileLayers;
            for (const std::shared_ptr<vt::Tile>& tile : tiles) {
                if (tile) {
                    tileLayers.insert(tileLayers.end(), tile->getLayers().begin(), tile->getLayers().end());
                }
            }

            auto tileMap = std::make_shared<TileMap>();
            (*tileMap)[0] = std::make_shared<vt::Tile>(targetTile, tileLayers);
            return tileMap;
        } catch (const std::exception& ex) {
            Log::Errorf("CartoVectorTileDecoder::decodeTile: Exception while decoding: %s", ex.what());
        }
        return std::shared_ptr<TileMap>();
    }

    void CartoVectorTileDecoder::updateLayerStyle(const std::string& layerId, const std::string& cartoCSS) {
        std::shared_ptr<mvt::Map> map;
        try {
            auto assetLoader = std::make_shared<CartoCSSAssetLoader>("", _assetPackage);
            css::CartoCSSMapLoader mapLoader(assetLoader, _logger);
            mapLoader.setIgnoreLayerPredicates(true);
            map = mapLoader.loadMap(cartoCSS);
        }
        catch (const std::exception& ex) {
            throw ParseException("CartoCSS style parsing failed", ex.what());
        }

        if (!_layerIds.empty() && _layerIds.front() == layerId) {
            _backgroundColor = Color(map->getSettings().backgroundColor.value());

            std::shared_ptr<const vt::BitmapPattern> backgroundPattern;
            if (!map->getSettings().backgroundImage.empty()) {
                auto bitmapLoader = std::make_shared<VTBitmapLoader>("", _assetPackage);
                auto bitmapManager = std::make_shared<vt::BitmapManager>(bitmapLoader);
                backgroundPattern = bitmapManager->loadBitmapPattern(map->getSettings().backgroundImage, 1.0f, 1.0f);
            }
            _backgroundPattern = backgroundPattern;
        }

        _layerStyles[layerId] = cartoCSS;
        _layerMaps[layerId] = map;
    }
    
    const int CartoVectorTileDecoder::DEFAULT_TILE_SIZE = 256;
    const int CartoVectorTileDecoder::STROKEMAP_SIZE = 512;
    const int CartoVectorTileDecoder::GLYPHMAP_SIZE = 2048;
}
