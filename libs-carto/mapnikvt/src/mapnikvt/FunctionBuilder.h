/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_FUNCTIONBUILDER_H_
#define _CARTO_MAPNIKVT_FUNCTIONBUILDER_H_

#include <string>
#include <map>
#include <unordered_map>

#include "ParserUtils.h"
#include "GeneratorUtils.h"
#include "vt/Styles.h"

namespace carto { namespace mvt {
    class FunctionBuilder final {
    public:
        std::shared_ptr<const vt::FloatFunction> createFloatFunction(float value) const {
            auto it = _floatFunctionCache.find(value);
            if (it != _floatFunctionCache.end()) {
                return it->second;
            }

            auto floatFunc = std::make_shared<vt::FloatFunction>([value](const vt::ViewState&) {
                return value;
            });

            if (_floatFunctionCache.size() >= MAX_CACHE_SIZE) {
                _floatFunctionCache.erase(_floatFunctionCache.begin());
            }
            _floatFunctionCache[value] = floatFunc;
            return floatFunc;
        }

        std::shared_ptr<const vt::ColorFunction> createColorFunction(const vt::Color& value) const {
            auto it = _colorFunctionCache.find(value);
            if (it != _colorFunctionCache.end()) {
                return it->second;
            }

            auto colorFunc = std::make_shared<vt::ColorFunction>([value](const vt::ViewState&) {
                return value;
            });

            if (_colorFunctionCache.size() >= MAX_CACHE_SIZE) {
                _colorFunctionCache.erase(_colorFunctionCache.begin());
            }
            _colorFunctionCache[value] = colorFunc;
            return colorFunc;
        }

        std::shared_ptr<const vt::FloatFunction> createChainedFloatFunction(const std::string& name, const std::function<float(float)>& func, const std::shared_ptr<const vt::FloatFunction>& baseFunc) const {
            if (!baseFunc) {
                return baseFunc;
            }
            
            auto it = _chainedFloatFunctionCache.find(std::make_pair(name, baseFunc));
            if (it != _chainedFloatFunctionCache.end()) {
                return it->second;
            }

            auto chainedFunc = std::make_shared<vt::FloatFunction>([func, baseFunc](const vt::ViewState& viewState) {
                return func((*baseFunc)(viewState));
            });

            if (_chainedFloatFunctionCache.size() >= MAX_CACHE_SIZE) {
                _chainedFloatFunctionCache.erase(_chainedFloatFunctionCache.begin());
            }
            _chainedFloatFunctionCache[std::make_pair(name, baseFunc)] = chainedFunc;
            return chainedFunc;
        }

        std::shared_ptr<const vt::ColorFunction> createColorOpacityFunction(const std::shared_ptr<const vt::ColorFunction>& color, const std::shared_ptr<const vt::FloatFunction>& opacity) {
            if (!opacity) {
                return color;
            }

            auto it = _colorOpacityFunctionCache.find(std::make_pair(color, opacity));
            if (it != _colorOpacityFunctionCache.end()) {
                return it->second;
            }

            auto colorOpacityFunc = std::make_shared<vt::ColorFunction>([color, opacity](const vt::ViewState& viewState) {
                return vt::Color::fromColorOpacity((*color)(viewState), (*opacity)(viewState));
            });

            if (_colorOpacityFunctionCache.size() >= MAX_CACHE_SIZE) {
                _colorOpacityFunctionCache.erase(_colorOpacityFunctionCache.begin());
            }
            _colorOpacityFunctionCache[std::make_pair(color, opacity)] = colorOpacityFunc;
            return colorOpacityFunc;
        }

    private:
        constexpr static std::size_t MAX_CACHE_SIZE = 256;

        mutable std::unordered_map<float, std::shared_ptr<const vt::FloatFunction>> _floatFunctionCache;
        mutable std::unordered_map<vt::Color, std::shared_ptr<const vt::ColorFunction>> _colorFunctionCache;
        mutable std::map<std::pair<std::string, std::shared_ptr<const vt::FloatFunction>>, std::shared_ptr<const vt::FloatFunction>> _chainedFloatFunctionCache;
        mutable std::map<std::pair<std::shared_ptr<const vt::ColorFunction>, std::shared_ptr<const vt::FloatFunction>>, std::shared_ptr<const vt::ColorFunction>> _colorOpacityFunctionCache;
    };
} }

#endif
