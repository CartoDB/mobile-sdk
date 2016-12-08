#include "TorqueTileReader.h"
#include "TorqueLayer.h"
#include "TorqueFeatureDecoder.h"

namespace carto { namespace mvt {
    std::shared_ptr<FeatureDecoder::FeatureIterator> TorqueTileReader::createFeatureIterator(const std::shared_ptr<const Layer>& layer, const std::shared_ptr<const Style>& style, const FeatureExpressionContext& exprContext) const {
        int frameOffset = 0;
        if (auto torqueLayer = std::dynamic_pointer_cast<const TorqueLayer>(layer)) {
            frameOffset = torqueLayer->getFrameOffset();
        }
        int frame = _frame - frameOffset;
        if (_loop) {
            int frameCount = std::dynamic_pointer_cast<const TorqueMap>(_map)->getTorqueSettings().frameCount;
            if (frame >= 0 && frameCount > 0) {
                frame = frame % frameCount;
            }
        }

        return _featureDecoder.createFrameFeatureIterator(frame);
    }
} }
