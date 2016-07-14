#include "MBVectorTileDecoder.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "graphics/Bitmap.h"
#include "styles/CompiledStyleSet.h"
#include "styles/CartoCSSStyleSet.h"
#include "vectortiles/utils/MapnikVTLogger.h"
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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace carto {
    
    MBVectorTileDecoder::MBVectorTileDecoder(const std::shared_ptr<CompiledStyleSet>& compiledStyleSet) :
        _buffer(0),
        _cartoCSSLayerNamesIgnored(false),
        _layerNameOverride(),
        _logger(std::make_shared<MapnikVTLogger>("MBVectorTileDecoder")),
        _map(),
        _parameterValueMap(),
        _backgroundPattern(),
        _symbolizerContext(),
        _styleSet(compiledStyleSet)
    {
        if (!compiledStyleSet) {
            throw NullArgumentException("Null compiledStyleSet");
        }

        updateCurrentStyle(compiledStyleSet);
    }
    
    MBVectorTileDecoder::MBVectorTileDecoder(const std::shared_ptr<CartoCSSStyleSet>& cartoCSSStyleSet) :
        _buffer(0),
        _cartoCSSLayerNamesIgnored(false),
        _layerNameOverride(),
        _logger(std::make_shared<MapnikVTLogger>("MBVectorTileDecoder")),
        _map(),
        _parameterValueMap(),
        _backgroundPattern(),
        _symbolizerContext(),
        _styleSet(cartoCSSStyleSet)
    {
        if (!cartoCSSStyleSet) {
            throw NullArgumentException("Null cartoCSSStyleSet");
        }

        updateCurrentStyle(cartoCSSStyleSet);
    }
    
    MBVectorTileDecoder::~MBVectorTileDecoder() {
    }
        
    std::shared_ptr<CompiledStyleSet> MBVectorTileDecoder::getCompiledStyleSet() const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (auto compiledStyleSet = boost::get<std::shared_ptr<CompiledStyleSet> >(&_styleSet)) {
            return *compiledStyleSet;
        }
        return std::shared_ptr<CompiledStyleSet>();
    }
    
    void MBVectorTileDecoder::setCompiledStyleSet(const std::shared_ptr<CompiledStyleSet>& styleSet) {
        if (!styleSet) {
            throw NullArgumentException("Null styleSet");
        }

        updateCurrentStyle(styleSet);
        notifyDecoderChanged();
    }

    std::shared_ptr<CartoCSSStyleSet> MBVectorTileDecoder::getCartoCSSStyleSet() const {
        std::lock_guard<std::mutex> lock(_mutex);

        if (auto cartoCSSStyleSet = boost::get<std::shared_ptr<CartoCSSStyleSet> >(&_styleSet)) {
            return *cartoCSSStyleSet;
        }
        return std::shared_ptr<CartoCSSStyleSet>();
    }
    
    void MBVectorTileDecoder::setCartoCSSStyleSet(const std::shared_ptr<CartoCSSStyleSet>& styleSet) {
        if (!styleSet) {
            throw NullArgumentException("Null styleSet");
        }

        updateCurrentStyle(styleSet);
        notifyDecoderChanged();
    }

    std::vector<std::string> MBVectorTileDecoder::getStyleParameters() const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        std::vector<std::string> params;
        for (auto it = _map->getNutiParameterMap().begin(); it != _map->getNutiParameterMap().end(); it++) {
            params.push_back(it->first);
        }
        return params;
    }

    std::string MBVectorTileDecoder::getStyleParameter(const std::string& param) const {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _map->getNutiParameterMap().find(param);
        if (it == _map->getNutiParameterMap().end()) {
            throw InvalidArgumentException("Could not find parameter");
        }
        const mvt::NutiParameter& nutiParam = it->second;
        
        mvt::Value value = nutiParam.getDefaultValue();
        {
            auto it2 = _parameterValueMap->find(param);
            if (it2 != _parameterValueMap->end()) {
                value = it2->second;
            }
        }

        if (!nutiParam.getEnumMap().empty()) {
            for (auto it2 = nutiParam.getEnumMap().begin(); it2 != nutiParam.getEnumMap().end(); it2++) {
                if (it2->second == value) {
                    return it2->first;
                }
            }
        }
        else {
            if (auto val = boost::get<bool>(&value)) {
                return boost::lexical_cast<std::string>(*val);
            }
            else if (auto val = boost::get<long long>(&value)) {
                return boost::lexical_cast<std::string>(*val);
            }
            else if (auto val = boost::get<double>(&value)) {
                return boost::lexical_cast<std::string>(*val);
            }
            else if (auto val = boost::get<std::string>(&value)) {
                return *val;
            }
        }
        return std::string();
    }

    void MBVectorTileDecoder::setStyleParameter(const std::string& param, const std::string& value) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
    
            auto it = _map->getNutiParameterMap().find(param);
            if (it == _map->getNutiParameterMap().end()) {
                throw InvalidArgumentException("Could not find parameter");
            }
            const mvt::NutiParameter& nutiParam = it->second;

            if (!nutiParam.getEnumMap().empty()) {
                auto it2 = nutiParam.getEnumMap().find(boost::lexical_cast<std::string>(value));
                if (it2 == nutiParam.getEnumMap().end()) {
                    throw InvalidArgumentException("Illegal enum value for parameter");
                }
                (*_parameterValueMap)[param] = it2->second;
            } else {
                mvt::Value val = nutiParam.getDefaultValue();
                if (boost::get<bool>(&val)) {
                    if (value == "true") {
                        val = mvt::Value(true);
                    }
                    else if (value == "false") {
                        val = mvt::Value(false);
                    }
                    else {
                        val = mvt::Value(boost::lexical_cast<bool>(value));
                    }
                }
                else if (boost::get<long long>(&val)) {
                    val = mvt::Value(boost::lexical_cast<long long>(value));
                }
                else if (boost::get<double>(&val)) {
                    val = mvt::Value(boost::lexical_cast<double>(value));
                }
                else if (boost::get<std::string>(&val)) {
                    val = value;
                }
                (*_parameterValueMap)[param] = val;
            }
    
            mvt::SymbolizerContext::Settings settings(DEFAULT_TILE_SIZE, *_parameterValueMap);
            _symbolizerContext = std::make_shared<mvt::SymbolizerContext>(_symbolizerContext->getBitmapManager(), _symbolizerContext->getFontManager(), _symbolizerContext->getStrokeMap(), _symbolizerContext->getGlyphMap(), settings);
        }
        notifyDecoderChanged();
    }

    float MBVectorTileDecoder::getBuffering() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _buffer;
    }
        
    void MBVectorTileDecoder::setBuffering(float buffer) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _buffer = buffer;
        }
        notifyDecoderChanged();
    }

    bool MBVectorTileDecoder::isCartoCSSLayerNamesIgnored() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cartoCSSLayerNamesIgnored;
    }

    void MBVectorTileDecoder::setCartoCSSLayerNamesIgnored(bool ignore) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _cartoCSSLayerNamesIgnored = ignore;
        }

        notifyDecoderChanged();
    }
        
    std::string MBVectorTileDecoder::getLayerNameOverride() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _layerNameOverride;
    }

    void MBVectorTileDecoder::setLayerNameOverride(const std::string& name) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _layerNameOverride = name;
        }
        notifyDecoderChanged();
    }

    Color MBVectorTileDecoder::getBackgroundColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return Color(_map->getSettings().backgroundColor.value());
    }
    
    std::shared_ptr<const vt::BitmapPattern> MBVectorTileDecoder::getBackgroundPattern() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _backgroundPattern;
    }
        
    int MBVectorTileDecoder::getMinZoom() const {
        return 0;
    }
    
    int MBVectorTileDecoder::getMaxZoom() const {
        return Const::MAX_SUPPORTED_ZOOM_LEVEL;
    }
        
    std::shared_ptr<MBVectorTileDecoder::TileMap> MBVectorTileDecoder::decodeTile(const vt::TileId& tile, const vt::TileId& targetTile, const std::shared_ptr<BinaryData>& tileData) const {
        if (!tileData) {
            Log::Warn("MBVectorTileDecoder::decodeTile: Null tile data");
            return std::shared_ptr<TileMap>();
        }
        if (tileData->empty()) {
            return std::shared_ptr<TileMap>();
        }

        std::shared_ptr<mvt::Map> map;
        std::shared_ptr<mvt::SymbolizerContext> symbolizerContext;
        float buffer;
        std::string layerNameOverride;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            map = _map;
            symbolizerContext = _symbolizerContext;
            buffer = _buffer;
            layerNameOverride = _layerNameOverride;
        }
    
        try {
            mvt::MBVTFeatureDecoder decoder(*tileData->getDataPtr(), _logger);
            decoder.setTransform(calculateTileTransform(tile, targetTile));
            decoder.setBuffer(buffer);
            
            mvt::MBVTTileReader reader(map, *symbolizerContext, decoder);
            reader.setLayerNameOverride(layerNameOverride);

            if (std::shared_ptr<vt::Tile> tile = reader.readTile(targetTile)) {
                auto tileMap = std::make_shared<TileMap>();
                (*tileMap)[0] = tile;
                return tileMap;
            }
        } catch (const std::exception& ex) {
            Log::Errorf("MBVectorTileDecoder::decodeTile: Exception while decoding: %s", ex.what());
        }
        return std::shared_ptr<TileMap>();
    }

    void MBVectorTileDecoder::updateCurrentStyle(const boost::variant<std::shared_ptr<CompiledStyleSet>, std::shared_ptr<CartoCSSStyleSet> >& styleSet) {
        std::lock_guard<std::mutex> lock(_mutex);

        std::string styleAssetName;
        std::shared_ptr<AssetPackage> styleSetData;
        std::shared_ptr<mvt::Map> map;

        if (auto cartoCSSStyleSet = boost::get<std::shared_ptr<CartoCSSStyleSet> >(&styleSet)) {
            styleAssetName = "";
            styleSetData = (*cartoCSSStyleSet)->getAssetPackage();

            try {
                auto assetLoader = std::make_shared<CartoCSSAssetLoader>("", (*cartoCSSStyleSet)->getAssetPackage());
                css::CartoCSSMapLoader mapLoader(assetLoader, _logger);
                mapLoader.setIgnoreLayerPredicates(_cartoCSSLayerNamesIgnored);
                map = mapLoader.loadMap((*cartoCSSStyleSet)->getCartoCSS());
            }
            catch (const std::exception& ex) {
                throw ParseException("CartoCSS style parsing failed", ex.what());
            }
        }
        else if (auto compiledStyleSet = boost::get<std::shared_ptr<CompiledStyleSet> >(&styleSet)) {
            styleAssetName = (*compiledStyleSet)->getStyleAssetName();
            if (styleAssetName.empty()) {
                throw InvalidArgumentException("Could not find any styles in the style set");
            }

            styleSetData = (*compiledStyleSet)->getAssetPackage();

            std::shared_ptr<BinaryData> styleData;
            if (styleSetData) {
                styleData = styleSetData->loadAsset(styleAssetName);
            }
            if (!styleData) {
                throw GenericException("Failed to load style description asset");
            }

            if (boost::algorithm::ends_with(styleAssetName, ".xml")) {
                pugi::xml_document doc;
                if (!doc.load_buffer(styleData->data(), styleData->size())) {
                    throw ParseException("Style element XML parsing failed");
                }
                try {
                    auto symbolizerParser = std::make_shared<mvt::SymbolizerParser>(_logger);
                    mvt::MapParser mapParser(symbolizerParser, _logger);
                    map = mapParser.parseMap(doc);
                }
                catch (const std::exception& ex) {
                    throw ParseException("XML style processing failed", ex.what());
                }
            }
            else if (boost::algorithm::ends_with(styleAssetName, ".json")) {
                try {
                    auto assetLoader = std::make_shared<CartoCSSAssetLoader>(FileUtils::GetFilePath(styleAssetName), styleSetData);
                    css::CartoCSSMapLoader mapLoader(assetLoader, _logger);
                    mapLoader.setIgnoreLayerPredicates(_cartoCSSLayerNamesIgnored);
                    map = mapLoader.loadMapProject(styleAssetName);
                }
                catch (const std::exception& ex) {
                    throw ParseException("CartoCSS style parsing failed", ex.what());
                }
            }
            else {
                throw GenericException("Failed to detect style asset type");
            }
        } else {
            throw InvalidArgumentException("Invalid style set");
        }

        auto parameterValueMap = std::make_shared<std::map<std::string, mvt::Value> >();
        for (auto it = map->getNutiParameterMap().begin(); it != map->getNutiParameterMap().end(); it++) {
            (*parameterValueMap)[it->first] = it->second.getDefaultValue();
        }

        mvt::SymbolizerContext::Settings settings(DEFAULT_TILE_SIZE, *parameterValueMap);
        auto fontManager = std::make_shared<vt::FontManager>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
        auto bitmapLoader = std::make_shared<VTBitmapLoader>(FileUtils::GetFilePath(styleAssetName), styleSetData);
        auto bitmapManager = std::make_shared<vt::BitmapManager>(bitmapLoader);
        auto strokeMap = std::make_shared<vt::StrokeMap>(STROKEMAP_SIZE);
        auto glyphMap = std::make_shared<vt::GlyphMap>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
        auto symbolizerContext = std::make_shared<mvt::SymbolizerContext>(bitmapManager, fontManager, strokeMap, glyphMap, settings);

        if (styleSetData) {
            std::string fontPrefix = map->getSettings().fontDirectory;
            fontPrefix = FileUtils::NormalizePath(FileUtils::GetFilePath(styleAssetName) + fontPrefix + "/");

            for (const std::string& assetName : styleSetData->getAssetNames()) {
                if (assetName.size() > fontPrefix.size() && assetName.substr(0, fontPrefix.size()) == fontPrefix) {
                    if (std::shared_ptr<BinaryData> fontData = styleSetData->loadAsset(assetName)) {
                        fontManager->loadFontData(*fontData->getDataPtr());
                    }
                }
            }
        }

        std::shared_ptr<const vt::BitmapPattern> backgroundPattern;
        if (!map->getSettings().backgroundImage.empty()) {
            backgroundPattern = bitmapManager->loadBitmapPattern(map->getSettings().backgroundImage, 1.0f, 1.0f);
        }

        _map = map;
        _parameterValueMap = parameterValueMap;
        _backgroundPattern = backgroundPattern;
        _symbolizerContext = symbolizerContext;
        _styleSet = styleSet;
    }
    
    const int MBVectorTileDecoder::DEFAULT_TILE_SIZE = 256;
    const int MBVectorTileDecoder::STROKEMAP_SIZE = 512;
    const int MBVectorTileDecoder::GLYPHMAP_SIZE = 2048;
}
