/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_TORQUETILEREADER_H_
#define _CARTO_MAPNIKVT_TORQUETILEREADER_H_

#include "TileReader.h"
#include "TorqueMap.h"

namespace carto { namespace mvt {
    class TorqueFeatureDecoder;
    
    class TorqueTileReader : public TileReader {
    public:
        explicit TorqueTileReader(std::shared_ptr<TorqueMap> map, int frame, bool loop, const SymbolizerContext& symbolizerContext, const TorqueFeatureDecoder& featureDecoder) : TileReader(std::move(map), symbolizerContext), _frame(frame), _loop(loop), _featureDecoder(featureDecoder) { }

    protected:
        virtual std::string getLayerName(const std::shared_ptr<const Layer>& layer) const override;

        virtual std::shared_ptr<FeatureDecoder::FeatureIterator> createFeatureIterator(const std::shared_ptr<const Layer>& layer, const std::shared_ptr<const Style>& style, const FeatureExpressionContext& exprContext) const override;

    private:
        const int _frame;
        const bool _loop;
        const TorqueFeatureDecoder& _featureDecoder;
    };
} }

#endif
