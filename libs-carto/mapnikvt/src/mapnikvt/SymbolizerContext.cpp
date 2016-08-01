#include "SymbolizerContext.h"
#include "PointSymbolizer.h"
#include "LineSymbolizer.h"
#include "LinePatternSymbolizer.h"
#include "PolygonSymbolizer.h"
#include "PolygonPatternSymbolizer.h"
#include "BuildingSymbolizer.h"
#include "MarkersSymbolizer.h"
#include "TextSymbolizer.h"
#include "ShieldSymbolizer.h"

namespace carto { namespace mvt {
    SymbolizerContext::Settings::Settings(float tileSize, std::map<std::string, Value> nutiParameterValueMap) :
        _tileSize(tileSize), _geometryScale(1.0f), _fontScale(1.0f), _zoomLevelBias(0.0f), _nutiParameterValueMap(std::move(nutiParameterValueMap))
    {
        auto geometryScaleIt = _nutiParameterValueMap.find("_geometryscale");
        if (geometryScaleIt != _nutiParameterValueMap.end()) {
            _geometryScale = static_cast<float>(boost::get<double>(geometryScaleIt->second));
        }

        auto fontScaleIt = _nutiParameterValueMap.find("_fontscale");
        if (fontScaleIt != _nutiParameterValueMap.end()) {
            _fontScale = static_cast<float>(boost::get<double>(fontScaleIt->second));
        }
        
        auto zoomLevelBiasIt = _nutiParameterValueMap.find("_zoomlevelbias");
        if (zoomLevelBiasIt != _nutiParameterValueMap.end()) {
            _zoomLevelBias = static_cast<float>(boost::get<double>(zoomLevelBiasIt->second));
        }
    }
} }
