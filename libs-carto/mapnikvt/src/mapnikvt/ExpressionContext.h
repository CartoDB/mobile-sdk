/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_EXPRESSIONCONTEXT_H_
#define _CARTO_MAPNIKVT_EXPRESSIONCONTEXT_H_

#include "Value.h"
#include "vt/TileId.h"

#include <map>
#include <memory>

namespace carto { namespace mvt {
    class Expression;
    class FeatureData;

    class ExpressionContext {
    public:
        virtual ~ExpressionContext() = default;
        
        virtual Value getVariable(const std::string& name) const = 0;
    };
    
    class FeatureExpressionContext : public ExpressionContext {
    public:
        FeatureExpressionContext();

        void setTileId(const vt::TileId& tileId) { _tileId = tileId; }
        const vt::TileId& getTileId() const { return _tileId; }

        void setAdjustedZoom(int zoom);
        int getAdjustedZoom() const { return _adjustedZoom; }
        float getScaleDenominator() const { return _scaleDenom; }

        void setFeatureData(std::shared_ptr<const FeatureData> featureData) { _featureData = std::move(featureData); }
        const std::shared_ptr<const FeatureData>& getFeatureData() const { return _featureData; }

        void setNutiParameterValueMap(std::map<std::string, Value> paramValueMap) { _nutiParameterValueMap = std::move(paramValueMap); }
        const std::map<std::string, Value>& getNutiParameterValueMap() const { return _nutiParameterValueMap; }

        virtual Value getVariable(const std::string& name) const override;

    private:
        vt::TileId _tileId = vt::TileId { 0, 0, 0 };
        int _adjustedZoom = 0;
        float _scaleDenom = 0;
        std::shared_ptr<const FeatureData> _featureData;
        std::map<std::string, Value> _nutiParameterValueMap;
    };

    class ViewExpressionContext : public ExpressionContext {
    public:
        ViewExpressionContext();

        void setZoom(float zoom);

        virtual Value getVariable(const std::string& name) const override;

        static bool isViewVariable(const std::string& name);

    private:
        float _zoom = 0;
    };
} }

#endif
