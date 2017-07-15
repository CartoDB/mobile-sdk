#include "MBVTTileReader.h"
#include "MBVTFeatureDecoder.h"
#include "ValueConverter.h"
#include "Expression.h"
#include "ExpressionContext.h"

namespace carto { namespace mvt {
    void MBVTTileReader::setLayerNameOverride(const std::string& name) {
        _layerNameOverride = name;
    }

    std::shared_ptr<FeatureDecoder::FeatureIterator> MBVTTileReader::createFeatureIterator(const std::shared_ptr<const Layer>& layer) const {
        std::string layerName = _layerNameOverride.empty() ? layer->getName() : _layerNameOverride;
        return _featureDecoder.createLayerFeatureIterator(layerName);
    }
} }
