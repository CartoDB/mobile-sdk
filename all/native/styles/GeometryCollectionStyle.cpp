#include "GeometryCollectionStyle.h"
#include "PointStyle.h"
#include "LineStyle.h"
#include "PolygonStyle.h"

namespace carto {
    
    GeometryCollectionStyle::GeometryCollectionStyle(const std::shared_ptr<PointStyle>& pointStyle, const std::shared_ptr<LineStyle>& lineStyle, const std::shared_ptr<PolygonStyle>& polygonStyle) :
        Style(Color()),
        _pointStyle(pointStyle),
        _lineStyle(lineStyle),
        _polygonStyle(polygonStyle)
    {
    }
    
    GeometryCollectionStyle::~GeometryCollectionStyle() {
    }
    
    std::shared_ptr<PointStyle> GeometryCollectionStyle::getPointStyle() const {
        return _pointStyle;
    }

    std::shared_ptr<LineStyle> GeometryCollectionStyle::getLineStyle() const {
        return _lineStyle;
    }
    
    std::shared_ptr<PolygonStyle> GeometryCollectionStyle::getPolygonStyle() const {
        return _polygonStyle;
    }
    
}
