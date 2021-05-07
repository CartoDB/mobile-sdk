#include "NMLModelStyleBuilder.h"
#include "assets/DefaultNMLModelNML.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "styles/NMLModelStyle.h"

namespace carto {

    NMLModelStyleBuilder::NMLModelStyleBuilder() :
        BillboardStyleBuilder(),
        _orientationMode(BillboardOrientation::BILLBOARD_ORIENTATION_GROUND),
        _scalingMode(BillboardScaling::BILLBOARD_SCALING_WORLD_SIZE),
        _modelAsset(GetDefaultModelAsset())
    {
        // Use different defaults for backward compatibility reasons
        _causesOverlap = false;
        _hideIfOverlapped = false;
        _scaleWithDPI = false;
    }
    
    NMLModelStyleBuilder::~NMLModelStyleBuilder() {
    }
        
    BillboardOrientation::BillboardOrientation NMLModelStyleBuilder::getOrientationMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _orientationMode;
    }
        
    void NMLModelStyleBuilder::setOrientationMode(BillboardOrientation::BillboardOrientation orientationMode) {
        std::lock_guard<std::mutex> lock(_mutex);
        _orientationMode = orientationMode;
    }
        
    BillboardScaling::BillboardScaling NMLModelStyleBuilder::getScalingMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _scalingMode;
    }
        
    void NMLModelStyleBuilder::setScalingMode(BillboardScaling::BillboardScaling scalingMode) {
        std::lock_guard<std::mutex> lock(_mutex);
        _scalingMode = scalingMode;
    }

    std::shared_ptr<BinaryData> NMLModelStyleBuilder::getModelAsset() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _modelAsset;
    }
    
    void NMLModelStyleBuilder::setModelAsset(const std::shared_ptr<BinaryData>& modelAsset) {
        if (!modelAsset) {
            throw NullArgumentException("Null modelAsset");
        }

        std::lock_guard<std::mutex> lock(_mutex);
        _modelAsset = modelAsset;
    }
        
    std::shared_ptr<NMLModelStyle> NMLModelStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<NMLModelStyle>(_color,
                                               _attachAnchorPointX,
                                               _attachAnchorPointY,
                                               _causesOverlap,
                                               _hideIfOverlapped,
                                               _horizontalOffset,
                                               _verticalOffset,
                                               _placementPriority,
                                               _scaleWithDPI,
                                               _animationStyle,
                                               _orientationMode,
                                               _scalingMode,
                                               _modelAsset);
    }
    
    std::shared_ptr<BinaryData> NMLModelStyleBuilder::GetDefaultModelAsset() {
        std::lock_guard<std::mutex> lock(_DefaultNMLModelMutex);
        if (!_DefaultNMLModel) {
            _DefaultNMLModel = std::make_shared<BinaryData>(default_nmlmodel_nml, default_nmlmodel_nml_len);
        }
        return _DefaultNMLModel;
    }

    std::shared_ptr<BinaryData> NMLModelStyleBuilder::_DefaultNMLModel;
    std::mutex NMLModelStyleBuilder::_DefaultNMLModelMutex;
    
}
