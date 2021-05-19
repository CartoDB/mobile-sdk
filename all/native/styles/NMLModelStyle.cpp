#include "NMLModelStyle.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"

#include <nml/Package.h>

namespace carto {

    NMLModelStyle::NMLModelStyle(const Color& color,
                                 float attachAnchorPointX,
                                 float attachAnchorPointY,
                                 bool causesOverlap,
                                 bool hideIfOverlapped,
                                 float horizontalOffset,
                                 float verticalOffset,
                                 int placementPriority,
                                 bool scaleWithDPI,
                                 const std::shared_ptr<AnimationStyle>& animStyle,
                                 BillboardOrientation::BillboardOrientation orientationMode,
                                 BillboardScaling::BillboardScaling scalingMode,
                                 const std::shared_ptr<BinaryData>& modelAsset) :
        BillboardStyle(color, attachAnchorPointX, attachAnchorPointY, causesOverlap, hideIfOverlapped, horizontalOffset, verticalOffset, placementPriority, scaleWithDPI, animStyle),
        _orientationMode(orientationMode),
        _scalingMode(scalingMode),
        _modelAsset(modelAsset),
        _sourceModel()
    {
        if (!modelAsset) {
            throw NullArgumentException("Null modelAsset");
        }

        std::shared_ptr<std::vector<unsigned char> > data = modelAsset->getDataPtr();
        protobuf::message modelMsg(data->data(), data->size());
        _sourceModel = std::make_shared<nml::Model>(modelMsg);
    }
    
    NMLModelStyle::~NMLModelStyle() {
    }
    
    BillboardOrientation::BillboardOrientation NMLModelStyle::getOrientationMode() const {
        return _orientationMode;
    }
    
    BillboardScaling::BillboardScaling NMLModelStyle::getScalingMode() const {
        return _scalingMode;
    }

    std::shared_ptr<BinaryData> NMLModelStyle::getModelAsset() const {
        return _modelAsset;
    }

    std::shared_ptr<nml::Model> NMLModelStyle::getSourceModel() const {
        return _sourceModel;
    }
    
}
