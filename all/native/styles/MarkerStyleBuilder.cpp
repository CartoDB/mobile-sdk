#include "MarkerStyleBuilder.h"
#include "assets/DefaultMarkerPNG.h"
#include "graphics/Bitmap.h"
#include "styles/MarkerStyle.h"

namespace carto {

    MarkerStyleBuilder::MarkerStyleBuilder() :
        BillboardStyleBuilder(),
        _anchorPointX(0),
        _anchorPointY(-1),
        _bitmap(GetDefaultBitmap()),
        _orientationMode(BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA),
        _scalingMode(BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE),
        _size(-1)
    {
    }
    
    MarkerStyleBuilder::~MarkerStyleBuilder() {
    }
        
    float MarkerStyleBuilder::getAnchorPointX() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _anchorPointX;
    }
        
    void MarkerStyleBuilder::setAnchorPointX(float anchorPointX) {
        std::lock_guard<std::mutex> lock(_mutex);
        _anchorPointX = anchorPointX;
    }
        
    float MarkerStyleBuilder::getAnchorPointY() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _anchorPointY;
    }
    
    void MarkerStyleBuilder::setAnchorPointY(float anchorPointY) {
        std::lock_guard<std::mutex> lock(_mutex);
        _anchorPointY = anchorPointY;
    }
        
    void MarkerStyleBuilder::setAnchorPoint(float anchorPointX, float anchorPointY) {
        std::lock_guard<std::mutex> lock(_mutex);
        _anchorPointX = anchorPointX;
        _anchorPointY = anchorPointY;
    }
        
    std::shared_ptr<Bitmap> MarkerStyleBuilder::getBitmap() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bitmap;
    }
    
    void MarkerStyleBuilder::setBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        std::lock_guard<std::mutex> lock(_mutex);
        _bitmap = bitmap;
    }
        
    BillboardOrientation::BillboardOrientation  MarkerStyleBuilder::getOrientationMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _orientationMode;
    }
        
    void MarkerStyleBuilder::setOrientationMode(BillboardOrientation::BillboardOrientation orientationMode) {
        std::lock_guard<std::mutex> lock(_mutex);
        _orientationMode = orientationMode;
    }
        
    BillboardScaling::BillboardScaling  MarkerStyleBuilder::getScalingMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _scalingMode;
    }
        
    void MarkerStyleBuilder::setScalingMode(BillboardScaling::BillboardScaling scalingMode) {
        std::lock_guard<std::mutex> lock(_mutex);
        _scalingMode = scalingMode;
    }
        
    float  MarkerStyleBuilder::getSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _size;
    }
    
    void MarkerStyleBuilder::setSize(float size) {
        std::lock_guard<std::mutex> lock(_mutex);
        _size = size;
    }
    
    std::shared_ptr<MarkerStyle> MarkerStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::shared_ptr<MarkerStyle>(new MarkerStyle(_color,
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
                                                            _bitmap,
                                                            _orientationMode,
                                                            _scalingMode,
                                                            _size));
    }
    
    std::shared_ptr<Bitmap> MarkerStyleBuilder::GetDefaultBitmap() {
        std::lock_guard<std::mutex> lock(_DefaultBitmapMutex);
        if (!_DefaultBitmap) {
            _DefaultBitmap = Bitmap::CreateFromCompressed(default_marker_png, default_marker_png_len);
        }
        return _DefaultBitmap;
    }
    
    std::shared_ptr<Bitmap> MarkerStyleBuilder::_DefaultBitmap;
    std::mutex MarkerStyleBuilder::_DefaultBitmapMutex;
    
}
