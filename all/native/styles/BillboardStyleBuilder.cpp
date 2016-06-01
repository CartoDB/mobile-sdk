#include "BillboardStyleBuilder.h"

namespace carto {

    BillboardStyleBuilder::~BillboardStyleBuilder() {
    }
    
    float BillboardStyleBuilder::getAttachAnchorPointX() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _attachAnchorPointX;
    }
    
    void BillboardStyleBuilder::setAttachAnchorPointX(float attachAnchorPointX) {
        std::lock_guard<std::mutex> lock(_mutex);
        _attachAnchorPointX = attachAnchorPointX;
    }
        
    float BillboardStyleBuilder::getAttachAnchorPointY() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _attachAnchorPointY;
    }
    
    void BillboardStyleBuilder::setAttachAnchorPointY(float attachAnchorPointY) {
        std::lock_guard<std::mutex> lock(_mutex);
        _attachAnchorPointY = attachAnchorPointY;
    }
        
    void BillboardStyleBuilder::setAttachAnchorPoint(float attachAnchorPointX, float attachAnchorPointY) {
        std::lock_guard<std::mutex> lock(_mutex);
        _attachAnchorPointX = attachAnchorPointX;
        _attachAnchorPointY = attachAnchorPointY;
    }
        
    bool BillboardStyleBuilder::isCausesOverlap() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _causesOverlap;
    }
    
    void BillboardStyleBuilder::setCausesOverlap(bool causesOverlap) {
        std::lock_guard<std::mutex> lock(_mutex);
        _causesOverlap = causesOverlap;
    }
    
    bool BillboardStyleBuilder::isHideIfOverlapped() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _hideIfOverlapped;
    }
    
    void BillboardStyleBuilder::setHideIfOverlapped(bool hideIfOverlapped) {
        std::lock_guard<std::mutex> lock(_mutex);
        _hideIfOverlapped = hideIfOverlapped;
    }
    
    float BillboardStyleBuilder::getHorizontalOffset() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _horizontalOffset;
    }
    
    void BillboardStyleBuilder::setHorizontalOffset(float horizontalOffset) {
        std::lock_guard<std::mutex> lock(_mutex);
        _horizontalOffset = horizontalOffset;
    }
    
    int BillboardStyleBuilder::getPlacementPriority() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _placementPriority;
    }
    
    void BillboardStyleBuilder::setPlacementPriority(int placementPriority) {
        std::lock_guard<std::mutex> lock(_mutex);
        _placementPriority = placementPriority;
    }
    
    float BillboardStyleBuilder::getVerticalOffset() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _verticalOffset;
    }
    
    void BillboardStyleBuilder::setVerticalOffset(float verticalOffset) {
        std::lock_guard<std::mutex> lock(_mutex);
        _verticalOffset = verticalOffset;
    }
        
    bool BillboardStyleBuilder::isScaleWithDPI() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _scaleWithDPI;
    }
    
    void BillboardStyleBuilder::setScaleWithDPI(bool scaleWithDPI) {
        std::lock_guard<std::mutex> lock(_mutex);
        _scaleWithDPI = scaleWithDPI;
    }
    
    BillboardStyleBuilder::BillboardStyleBuilder() :
        StyleBuilder(),
        _attachAnchorPointX(0),
        _attachAnchorPointY(1),
        _causesOverlap(true),
        _hideIfOverlapped(false),
        _horizontalOffset(0),
        _verticalOffset(0),
        _placementPriority(0),
        _scaleWithDPI(true)
    {
    }
    
}
