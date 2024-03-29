#include "Point.h"
#include "components/Exceptions.h"
#include "datasources/VectorDataSource.h"
#include "geometry/PointGeometry.h"
#include "renderers/drawdatas/PointDrawData.h"

namespace carto {

    Point::Point(const std::shared_ptr<PointGeometry>& geometry, const std::shared_ptr<PointStyle>& style) :
        VectorElement(geometry),
        _drawData(),
        _style(style)
    {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
        
    Point::Point(const MapPos& pos, const std::shared_ptr<PointStyle>& style) :
        VectorElement(std::make_shared<PointGeometry>(pos)),
        _drawData(),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
    
    Point::~Point() {
    }
    
    std::shared_ptr<PointGeometry> Point::getGeometry() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return std::static_pointer_cast<PointGeometry>(_geometry);
    }
    
    void Point::setGeometry(const std::shared_ptr<PointGeometry>& geometry) {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _geometry = geometry;
        }
        notifyElementChanged();
    }
    
    MapPos Point::getPos() const {
        return getGeometry()->getPos();
    }
        
    void Point::setPos(const MapPos& pos) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _geometry = std::make_shared<PointGeometry>(pos);
        }
        notifyElementChanged();
    }
    
    std::shared_ptr<PointStyle> Point::getStyle() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _style;
    }
    
    void Point::setStyle(const std::shared_ptr<PointStyle>& style) {
        if (!style) {
            throw NullArgumentException("Null style");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<PointDrawData> Point::getDrawData() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _drawData;
    }
    
    void Point::setDrawData(const std::shared_ptr<PointDrawData>& drawData) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _drawData = drawData;
    }
    
}
