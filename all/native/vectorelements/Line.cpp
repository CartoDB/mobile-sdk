#include "Line.h"
#include "components/Exceptions.h"
#include "datasources/VectorDataSource.h"
#include "geometry/LineGeometry.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "utils/Log.h"

namespace carto {

    Line::Line(const std::shared_ptr<LineGeometry>& geometry, const std::shared_ptr<LineStyle>& style) :
        VectorElement(geometry),
        _style(style)
    {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
        
    Line::Line(const std::vector<MapPos>& poses, const std::shared_ptr<LineStyle>& style) :
        VectorElement(std::make_shared<LineGeometry>(poses)),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
    
    Line::~Line() {
    }
    
    std::shared_ptr<LineGeometry> Line::getGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::static_pointer_cast<LineGeometry>(_geometry);
    }
    
    void Line::setGeometry(const std::shared_ptr<LineGeometry>& geometry) {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = geometry;
        }
        notifyElementChanged();
    }
    
    std::vector<MapPos> Line::getPoses() const {
        return getGeometry()->getPoses();
    }
        
    void Line::setPoses(const std::vector<MapPos>& poses) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = std::make_shared<LineGeometry>(poses);
        }
        notifyElementChanged();
    
    }
    
    std::shared_ptr<LineStyle> Line::getStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _style;
    }
    
    void Line::setStyle(const std::shared_ptr<LineStyle>& style) {
        if (!style) {
            throw NullArgumentException("Null style");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<LineDrawData> Line::getDrawData() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _drawData;
    }
        
    void Line::setDrawData(const std::shared_ptr<LineDrawData>& drawData) {
        std::lock_guard<std::mutex> lock(_mutex);
        _drawData = drawData;
    }
    
}
