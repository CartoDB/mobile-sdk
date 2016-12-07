/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_MBVTTILEREADER_H_
#define _CARTO_MAPNIKVT_MBVTTILEREADER_H_

#include "TileReader.h"
#include "Map.h"

namespace carto { namespace mvt {
    class MBVTFeatureDecoder;
    
    class MBVTTileReader : public TileReader {
    public:
        explicit MBVTTileReader(std::shared_ptr<Map> map, const SymbolizerContext& symbolizerContext, const MBVTFeatureDecoder& featureDecoder) : TileReader(std::move(map), symbolizerContext), _featureDecoder(featureDecoder) { }

        void setLayerNameOverride(const std::string& name);
        void setLayerFilter(const std::string& layerId);

    protected:
        virtual std::string getLayerName(const std::shared_ptr<const Layer>& layer) const override;

        virtual std::shared_ptr<FeatureDecoder::FeatureIterator> createFeatureIterator(const std::shared_ptr<const Layer>& layer, const std::shared_ptr<const Style>& style, const FeatureExpressionContext& exprContext) const override;

        const MBVTFeatureDecoder& _featureDecoder;
        std::string _layerNameOverride;
        std::string _layerFilter;
    };
} }

#endif
