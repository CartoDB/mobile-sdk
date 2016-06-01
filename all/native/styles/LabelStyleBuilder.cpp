#include "LabelStyleBuilder.h"
#include "styles/LabelStyle.h"

namespace carto {

    LabelStyleBuilder::LabelStyleBuilder() :
        BillboardStyleBuilder(),
        _anchorPointX(0),
        _anchorPointY(-1),
        _flippable(true),
        _orientationMode(BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA),
        _scalingMode(BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE)
    {
    }
    
    LabelStyleBuilder::~LabelStyleBuilder() {
    }
        
    float LabelStyleBuilder::getAnchorPointX() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _anchorPointX;
    }
        
    void LabelStyleBuilder::setAnchorPointX(float anchorPointX) {
        std::lock_guard<std::mutex> lock(_mutex);
        _anchorPointX = anchorPointX;
    }
        
    float LabelStyleBuilder::getAnchorPointY() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _anchorPointY;
    }
    
    void LabelStyleBuilder::setAnchorPointY(float anchorPointY) {
        std::lock_guard<std::mutex> lock(_mutex);
        _anchorPointY = anchorPointY;
    }

    void LabelStyleBuilder::setAnchorPoint(float anchorPointX, float anchorPointY) {
        std::lock_guard<std::mutex> lock(_mutex);
        _anchorPointX = anchorPointX;
        _anchorPointY = anchorPointY;
    }
        
    bool LabelStyleBuilder::isFlippable() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _flippable;
    }
        
    void LabelStyleBuilder::setFlippable(bool flippable) {
        std::lock_guard<std::mutex> lock(_mutex);
        _flippable = flippable;
    }
        
    BillboardOrientation::BillboardOrientation LabelStyleBuilder::getOrientationMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _orientationMode;
    }
        
    void LabelStyleBuilder::setOrientationMode(BillboardOrientation::BillboardOrientation orientationMode) {
        std::lock_guard<std::mutex> lock(_mutex);
        _orientationMode = orientationMode;
    }
        
    BillboardScaling::BillboardScaling LabelStyleBuilder::getScalingMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _scalingMode;
    }
        
    void LabelStyleBuilder::setScalingMode(BillboardScaling::BillboardScaling scalingMode) {
        std::lock_guard<std::mutex> lock(_mutex);
        _scalingMode = scalingMode;
    }
    
    std::shared_ptr<LabelStyle> LabelStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::shared_ptr<LabelStyle>(new LabelStyle(_color,
                                                          _attachAnchorPointX,
                                                          _attachAnchorPointY,
                                                          _causesOverlap,
                                                          _hideIfOverlapped,
                                                          _horizontalOffset,
                                                          _verticalOffset,
                                                          _placementPriority,
                                                          _scaleWithDPI,
                                                          _anchorPointX,
                                                          _anchorPointY,
                                                          _flippable,
                                                          _orientationMode,
                                                          _scalingMode));
    }
        
}
