/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_MAP_H_
#define _CARTO_MAPNIKVT_MAP_H_

#include "FontSet.h"
#include "Layer.h"
#include "Style.h"
#include "Parameter.h"
#include "NutiParameter.h"
#include "vt/Color.h"

#include <memory>
#include <string>
#include <vector>
#include <map>

namespace carto { namespace mvt {
    class Map {
    public:
        struct Settings {
            vt::Color backgroundColor = vt::Color(0x00000000);
            std::string backgroundImage;
            std::string fontDirectory = "fonts";
        };
        
        explicit Map(const Settings& settings) : _settings(settings) { }
        virtual ~Map() = default;

        const Settings& getSettings() const { return _settings; }

        void setNutiParameters(const std::vector<NutiParameter>& nutiParameters);
        const std::map<std::string, NutiParameter>& getNutiParameterMap() const { return _nutiParameterMap; }
        
        void setParameters(const std::vector<Parameter>& parameters);
        const std::map<std::string, Parameter>& getParameterMap() const { return _parameterMap; }

        void clearStyles();
        void addStyle(const std::shared_ptr<Style>& style);
        const std::shared_ptr<Style>& getStyle(const std::string& name) const;
        const std::vector<std::shared_ptr<Style>>& getStyles() const { return _styles; }

        void clearFontSets();
        void addFontSet(const std::shared_ptr<FontSet>& fontSet);
        const std::shared_ptr<FontSet>& getFontSet(const std::string& name) const;
        const std::vector<std::shared_ptr<FontSet>>& getFontSets() const { return _fontSets; }

        void clearLayers();
        void addLayer(const std::shared_ptr<Layer>& layer);
        const std::shared_ptr<Layer>& getLayer(const std::string& name) const;
        const std::vector<std::shared_ptr<Layer>>& getLayers() const { return _layers; }

    private:
        Settings _settings;
        std::map<std::string, NutiParameter> _nutiParameterMap;
        std::map<std::string, Parameter> _parameterMap;
        std::vector<std::shared_ptr<Style>> _styles;
        std::map<std::string, std::shared_ptr<Style>> _styleMap;
        std::vector<std::shared_ptr<FontSet>> _fontSets;
        std::map<std::string, std::shared_ptr<FontSet>> _fontSetMap;
        std::vector<std::shared_ptr<Layer>> _layers;
        std::map<std::string, std::shared_ptr<Layer>> _layerMap;
    };
} }

#endif
