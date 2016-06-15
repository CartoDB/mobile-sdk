#include "MBVectorTileDecoder.h"
#include "core/BinaryData.h"
#include "graphics/Bitmap.h"
#include "vectortiles/AssetPackage.h"
#include "vectortiles/CompiledStyleSet.h"
#include "vectortiles/CartoCSSStyleSet.h"
#include "vectortiles/utils/MapnikVTLogger.h"
#include "vectortiles/utils/VTBitmapLoader.h"
#include "vectortiles/utils/CartoCSSAssetLoader.h"
#include "vt/Tile.h"
#include "Mapnikvt/Value.h"
#include "Mapnikvt/SymbolizerParser.h"
#include "Mapnikvt/SymbolizerContext.h"
#include "Mapnikvt/MBVTFeatureDecoder.h"
#include "Mapnikvt/MBVTTileReader.h"
#include "Mapnikvt/MapParser.h"
#include "cartocss/CartoCSSMapLoader.h"
#include "utils/FileUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace carto {
    
    MBVectorTileDecoder::MBVectorTileDecoder(const std::shared_ptr<CompiledStyleSet>& compiledStyleSet) :
        _buffer(0),
        _cartoCSSLayerNamesIgnored(false),
        _layerNameOverride(),
        _compiledStyleSet(compiledStyleSet),
        _cartoCSSStyleSet(),
        _styleSetData(compiledStyleSet->getAssetPackage())
    {
        _logger = std::make_shared<MapnikVTLogger>("MBVectorTileDecoder");

        if (!_compiledStyleSet->getStyleAssetName().empty()) {
            updateCurrentStyle();
        } else {
            Log::Error("MBVectorTileDecoder::MBVectorTileDecoder: Could not find any styles in the style set");
        }
    }
    
    MBVectorTileDecoder::MBVectorTileDecoder(const std::shared_ptr<CartoCSSStyleSet>& cartoCSSStyleSet) :
        _buffer(0),
        _cartoCSSLayerNamesIgnored(false),
        _layerNameOverride(),
        _compiledStyleSet(),
        _cartoCSSStyleSet(cartoCSSStyleSet),
        _styleSetData(cartoCSSStyleSet->getAssetPackage())
    {
        _logger = std::make_shared<MapnikVTLogger>("MBVectorTileDecoder");

        updateCurrentStyle();
    }
    
    MBVectorTileDecoder::~MBVectorTileDecoder() {
    }
        
    std::shared_ptr<CompiledStyleSet> MBVectorTileDecoder::getCompiledStyleSet() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _compiledStyleSet;
    }
    
    void MBVectorTileDecoder::setCompiledStyleSet(const std::shared_ptr<CompiledStyleSet>& styleSet) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _compiledStyleSet = styleSet;
            _cartoCSSStyleSet.reset();
            _styleSetData = styleSet->getAssetPackage();
            updateCurrentStyle();
        }

        notifyDecoderChanged();
    }

    std::shared_ptr<CartoCSSStyleSet> MBVectorTileDecoder::getCartoCSSStyleSet() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cartoCSSStyleSet;
    }
    
    void MBVectorTileDecoder::setCartoCSSStyleSet(const std::shared_ptr<CartoCSSStyleSet>& styleSet) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _cartoCSSStyleSet = styleSet;
            _compiledStyleSet.reset();
            _styleSetData = styleSet->getAssetPackage();
            updateCurrentStyle();
        }

        notifyDecoderChanged();
    }

    std::vector<std::string> MBVectorTileDecoder::getStyleParameters() const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_map) {
            return std::vector<std::string>();
        }
    
        std::vector<std::string> params;
        for (auto it = _map->getNutiParameterMap().begin(); it != _map->getNutiParameterMap().end(); it++) {
            params.push_back(it->first);
        }
        return params;
    }

    std::string MBVectorTileDecoder::getStyleParameter(const std::string& param) const {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_map) {
            Log::Errorf("MBVectorTileDecoder::getStyleParameter: Could not find parameter %s", param.c_str());
            return std::string();
        }

        auto it = _map->getNutiParameterMap().find(param);
        if (it == _map->getNutiParameterMap().end()) {
            Log::Errorf("MBVectorTileDecoder::getStyleParameter: Could not find parameter %s", param.c_str());
            return std::string();
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
    
            if (!_map) {
                return;
            }
    
            auto it = _map->getNutiParameterMap().find(param);
            if (it == _map->getNutiParameterMap().end()) {
                Log::Errorf("MBVectorTileDecoder::setStyleParameter: Could not find parameter %s", param.c_str());
                return;
            }
            const mvt::NutiParameter& nutiParam = it->second;

            if (!nutiParam.getEnumMap().empty()) {
                auto it2 = nutiParam.getEnumMap().find(boost::lexical_cast<std::string>(value));
                if (it2 == nutiParam.getEnumMap().end()) {
                    Log::Errorf("MBVectorTileDecoder::setStyleParameter: Illegal enum value for parameter %s: %s", param.c_str(), value.c_str());
                    return;
                }
                (*_parameterValueMap)[param] = it2->second;
            } else {
                try {
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
                catch (const boost::bad_numeric_cast&) {
                    Log::Errorf("MBVectorTileDecoder::setParameter: Could not convert parameter %s value: %s", param.c_str(), value.c_str());
        }
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
        if (!_map) {
            return Color(0);
        }
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
            Log::Error("MBVectorTileDecoder::decodeTile: Null tile data");
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
    
        if (!map || !symbolizerContext) {
            return std::shared_ptr<TileMap>();
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

    void MBVectorTileDecoder::updateCurrentStyle() {
        std::string styleAssetName;

        // CartoCSS is defined, use it
        if (_cartoCSSStyleSet) {
            Log::Info("MBVectorTileDecoder::updateCurrentStyle: Loading CartoCSS style");
            _map = loadCartoCSSMap(_cartoCSSStyleSet);
        }
        else if (_compiledStyleSet) {
            styleAssetName = _compiledStyleSet->getStyleAssetName();
            if (!styleAssetName.empty()) {
                if (std::shared_ptr<BinaryData> styleData = _styleSetData->loadAsset(styleAssetName)) {
                    if (boost::algorithm::ends_with(styleAssetName, ".xml")) {
                        Log::Info("MBVectorTileDecoder::updateCurrentStyle: Loading Mapnik style");
                        _map = loadMapnikMap(*styleData->getDataPtr());
                    }
                    else  if (boost::algorithm::ends_with(styleAssetName, ".json")) {
                        Log::Info("MBVectorTileDecoder::updateCurrentStyle: Loading CartoCSS project");
                        _map = loadCartoCSSMap(styleAssetName, _styleSetData);
                    }
                    else {
                        Log::Error("MBVectorTileDecoder::updateCurrentStyle: Could not load style element from style set");
                        return;
                    }
                }
            }
        }

        if (!_map) {
            return;
        }
    
        _parameterValueMap = std::make_shared<std::map<std::string, mvt::Value> >();
        for (auto it = _map->getNutiParameterMap().begin(); it != _map->getNutiParameterMap().end(); it++) {
            (*_parameterValueMap)[it->first] = it->second.getDefaultValue();
        }

        mvt::SymbolizerContext::Settings settings(DEFAULT_TILE_SIZE, *_parameterValueMap);
        auto fontManager = std::make_shared<vt::FontManager>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
        auto bitmapLoader = std::make_shared<VTBitmapLoader>(FileUtils::GetFilePath(styleAssetName), _styleSetData);
        auto bitmapManager = std::make_shared<vt::BitmapManager>(bitmapLoader);
        auto strokeMap = std::make_shared<vt::StrokeMap>(STROKEMAP_SIZE);
        auto glyphMap = std::make_shared<vt::GlyphMap>(GLYPHMAP_SIZE, GLYPHMAP_SIZE);
        _symbolizerContext = std::make_shared<mvt::SymbolizerContext>(bitmapManager, fontManager, strokeMap, glyphMap, settings);

        if (_styleSetData) {
            Log::Info("MBVectorTileDecoder::updateCurrentStyle: Loading fonts");
            std::string fontPrefix = _map->getSettings().fontDirectory;
            fontPrefix = FileUtils::NormalizePath(FileUtils::GetFilePath(styleAssetName) + fontPrefix + "/");
            for (const std::string& assetName : _styleSetData->getAssetNames()) {
                if (assetName.size() > fontPrefix.size() && assetName.substr(0, fontPrefix.size()) == fontPrefix) {
                    if (std::shared_ptr<BinaryData> fontData = _styleSetData->loadAsset(assetName)) {
                        fontManager->loadFontData(*fontData->getDataPtr());
                    }
                }
            }
        }

        if (!_map->getSettings().backgroundImage.empty()) {
            _backgroundPattern = bitmapManager->loadBitmapPattern(_map->getSettings().backgroundImage, 1.0f, 1.0f);
        }
    }
    
    std::shared_ptr<mvt::Map> MBVectorTileDecoder::loadMapnikMap(const std::vector<unsigned char>& styleData) {
        pugi::xml_document doc;
        if (!doc.load_buffer(styleData.data(), styleData.size())) {
            Log::Error("MBVectorTileDecoder::loadMapnikMap: Could not load style element XML");
            return std::shared_ptr<mvt::Map>();
        }
        try {
            auto symbolizerParser = std::make_shared<mvt::SymbolizerParser>(_logger);
            mvt::MapParser mapParser(symbolizerParser, _logger);
            return mapParser.parseMap(doc);
        }
        catch (const std::exception& ex) {
            Log::Errorf("MBVectorTileDecoder::loadMapnikMap: Mapnik style parsing failed: %s", ex.what());
            return std::shared_ptr<mvt::Map>();
        }
    }

    std::shared_ptr<mvt::Map> MBVectorTileDecoder::loadCartoCSSMap(const std::string& styleAssetName, const std::shared_ptr<AssetPackage>& styleSetData) {
        try {
            auto assetLoader = std::make_shared<CartoCSSAssetLoader>(FileUtils::GetFilePath(styleAssetName), styleSetData);
            css::CartoCSSMapLoader mapLoader(assetLoader, _logger);
            mapLoader.setIgnoreLayerPredicates(_cartoCSSLayerNamesIgnored);
            return mapLoader.loadMapProject(styleAssetName);
        }
        catch (const std::exception& ex) {
            Log::Errorf("MBVectorTileDecoder::loadCartoCSSMap: CartoCSS style parsing failed: %s", ex.what());
            return std::shared_ptr<mvt::Map>();
        }
    }
    
    std::shared_ptr<mvt::Map> MBVectorTileDecoder::loadCartoCSSMap(const std::shared_ptr<CartoCSSStyleSet>& styleSet) {
        try {
            auto assetLoader = std::make_shared<CartoCSSAssetLoader>("", styleSet->getAssetPackage());
            css::CartoCSSMapLoader mapLoader(assetLoader, _logger);
            mapLoader.setIgnoreLayerPredicates(_cartoCSSLayerNamesIgnored);
            return mapLoader.loadMap(styleSet->getCartoCSS());
        }
        catch (const std::exception& ex) {
            Log::Errorf("MBVectorTileDecoder::loadCartoCSSMap: CartoCSS style parsing failed: %s", ex.what());
            return std::shared_ptr<mvt::Map>();
        }
    }
    
    const int MBVectorTileDecoder::DEFAULT_TILE_SIZE = 256;
    const int MBVectorTileDecoder::STROKEMAP_SIZE = 512;
    const int MBVectorTileDecoder::GLYPHMAP_SIZE = 2048;
}
