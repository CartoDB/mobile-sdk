/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_TEXTSYMBOLIZER_H_
#define _CARTO_MAPNIKVT_TEXTSYMBOLIZER_H_

#include "Symbolizer.h"
#include "FontSet.h"
#include "Expression.h"

#include <vector>
#include <functional>

namespace carto { namespace mvt {
    class TextSymbolizer : public Symbolizer {
    public:
        explicit TextSymbolizer(std::vector<std::shared_ptr<FontSet>> fontSets, std::shared_ptr<Logger> logger) : Symbolizer(std::move(logger)), _fontSets(std::move(fontSets)) { }

        void setTextExpression(std::shared_ptr<Expression> textExpression);
        const std::shared_ptr<Expression>& getTextExpression() const;
        
        virtual void build(const FeatureCollection& featureCollection, const FeatureExpressionContext& exprContext, const SymbolizerContext& symbolizerContext, vt::TileLayerBuilder& layerBuilder) override;

    protected:
        virtual void bindParameter(const std::string& name, const std::string& value) override;

        std::string getTransformedText(const std::string& text) const;
        std::shared_ptr<vt::Font> getFont(const SymbolizerContext& symbolizerContext) const;
        cglib::bbox2<float> calculateTextSize(const std::shared_ptr<vt::Font>& font, const std::string& text, const vt::TextFormatter::Options& formatterOptions) const;
        vt::TextFormatter::Options getFormatterOptions(const SymbolizerContext& symbolizerContext) const;
        vt::LabelOrientation convertTextPlacement(const std::string& orientation) const;

        void buildFeatureCollection(const FeatureCollection& featureCollection, const SymbolizerContext& symbolizerContext, vt::LabelOrientation placement, float textSize, const std::function<void(long long localId, long long globalId, const boost::optional<vt::TileLayerBuilder::Vertex>& vertex, const vt::TileLayerBuilder::Vertices& vertices)>& addText);

        const std::vector<std::shared_ptr<FontSet>> _fontSets;
        std::shared_ptr<Expression> _textExpression;
        std::string _text;
        std::string _textTransform;
        std::string _faceName;
        std::string _fontSetName;
        std::string _placement = "point";
        float _size = 10.0f;
        float _spacing = 0.0f;
        vt::Color _fill = vt::Color(0xff000000);
        float _opacity = 1.0f;
        vt::Color _haloFill = vt::Color(0xffffffff);
        float _haloOpacity = 1.0f;
        float _haloRadius = 0.0f;
        float _orientation = 0.0f;
        bool _orientationDefined = false;
        float _dx = 0.0f;
        float _dy = 0.0f;
        float _minimumDistance = 0.0f;
        bool _allowOverlap = false;
        float _wrapWidth = 0.0f;
        bool _wrapBefore = false;
        float _characterSpacing = 0.0f;
        float _lineSpacing = 0.0f;
        std::string _horizontalAlignment = "auto";
        std::string _verticalAlignment = "auto";
    };
} }

#endif
