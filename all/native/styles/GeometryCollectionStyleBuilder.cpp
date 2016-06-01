#include "GeometryCollectionStyleBuilder.h"
#include "PointStyleBuilder.h"
#include "LineStyleBuilder.h"
#include "PolygonStyleBuilder.h"
#include "styles/GeometryCollectionStyle.h"

namespace carto {
    
    GeometryCollectionStyleBuilder::GeometryCollectionStyleBuilder() :
        StyleBuilder(),
        _pointStyle(),
        _lineStyle(),
        _polygonStyle()
    {
    }
    
    GeometryCollectionStyleBuilder::~GeometryCollectionStyleBuilder() {
    }
    
    std::shared_ptr<PointStyle> GeometryCollectionStyleBuilder::getPointStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _pointStyle;
    }
    
    void GeometryCollectionStyleBuilder::setPointStyle(const std::shared_ptr<PointStyle>& pointStyle) {
        std::lock_guard<std::mutex> lock(_mutex);
        _pointStyle = pointStyle;
    }
    
    std::shared_ptr<LineStyle> GeometryCollectionStyleBuilder::getLineStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _lineStyle;
    }
    
    void GeometryCollectionStyleBuilder::setLineStyle(const std::shared_ptr<LineStyle>& lineStyle) {
        std::lock_guard<std::mutex> lock(_mutex);
        _lineStyle = lineStyle;
    }
    
    std::shared_ptr<PolygonStyle> GeometryCollectionStyleBuilder::getPolygonStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _polygonStyle;
    }
    
    void GeometryCollectionStyleBuilder::setPolygonStyle(const std::shared_ptr<PolygonStyle>& polygonStyle) {
        std::lock_guard<std::mutex> lock(_mutex);
        _polygonStyle = polygonStyle;
    }
    
    std::shared_ptr<GeometryCollectionStyle> GeometryCollectionStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<GeometryCollectionStyle>(_pointStyle, _lineStyle, _polygonStyle);
    }
        
}
