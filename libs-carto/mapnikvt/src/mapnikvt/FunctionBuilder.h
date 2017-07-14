/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_FUNCTIONBUILDER_H_
#define _CARTO_MAPNIKVT_FUNCTIONBUILDER_H_

#include <string>
#include <unordered_map>

#include "ParserUtils.h"
#include "GeneratorUtils.h"
#include "vt/Styles.h"

namespace carto { namespace mvt {
    class FunctionBuilder final {
    public:
        vt::FloatFunction createFloatFunction(float value) const {
            return vt::FloatFunction(value);
        }

        vt::ColorFunction createColorFunction(const vt::Color& value) const {
            return vt::ColorFunction(value);
        }

        vt::FloatFunction createChainedFloatFunction(const std::string& name, const std::function<float(float)>& func, const vt::FloatFunction& baseFunc) const {
            auto it = _chainedFloatFunctionCache.find(std::make_pair(name, baseFunc));
            if (it != _chainedFloatFunctionCache.end()) {
                return it->second;
            }

            vt::FloatFunction chainedFunc(std::make_shared<std::function<float(const vt::ViewState&)>>([func, baseFunc](const vt::ViewState& viewState) {
                return func((baseFunc)(viewState));
            }));

            if (_chainedFloatFunctionCache.size() >= MAX_CACHE_SIZE) {
                _chainedFloatFunctionCache.erase(_chainedFloatFunctionCache.begin());
            }
            _chainedFloatFunctionCache[std::make_pair(name, baseFunc)] = chainedFunc;
            return chainedFunc;
        }

        vt::ColorFunction createColorOpacityFunction(const vt::ColorFunction& color, const vt::FloatFunction& opacity) {
            if (opacity == vt::FloatFunction(1)) {
                return color;
            }

            auto it = _colorOpacityFunctionCache.find(std::make_pair(color, opacity));
            if (it != _colorOpacityFunctionCache.end()) {
                return it->second;
            }

            vt::ColorFunction colorOpacityFunc(std::make_shared<std::function<vt::Color(const vt::ViewState&)>>([color, opacity](const vt::ViewState& viewState) {
                return vt::Color::fromColorOpacity((color)(viewState), (opacity)(viewState));
            }));

            if (_colorOpacityFunctionCache.size() >= MAX_CACHE_SIZE) {
                _colorOpacityFunctionCache.erase(_colorOpacityFunctionCache.begin());
            }
            _colorOpacityFunctionCache[std::make_pair(color, opacity)] = colorOpacityFunc;
            return colorOpacityFunc;
        }

    private:
        struct StringFloatFunctionPairHash {
            std::size_t operator() (const std::pair<std::string, vt::FloatFunction>& pair) const {
                return std::hash<std::string>()(pair.first) + std::hash<vt::FloatFunction>()(pair.second) * 2;
            }
        };

        struct ColorFunctionFloatFunctionPairHash {
            std::size_t operator() (const std::pair<vt::ColorFunction, vt::FloatFunction>& pair) const {
                return std::hash<vt::ColorFunction>()(pair.first) + std::hash<vt::FloatFunction>()(pair.second) * 2;
            }
        };

        constexpr static std::size_t MAX_CACHE_SIZE = 256;

        mutable std::unordered_map<std::pair<std::string, vt::FloatFunction>, vt::FloatFunction, StringFloatFunctionPairHash> _chainedFloatFunctionCache;
        mutable std::unordered_map<std::pair<vt::ColorFunction, vt::FloatFunction>, vt::ColorFunction, ColorFunctionFloatFunctionPairHash> _colorOpacityFunctionCache;
    };
} }

#endif
