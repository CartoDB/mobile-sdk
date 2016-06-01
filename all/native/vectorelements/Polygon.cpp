#include "Polygon.h"
#include "datasources/VectorDataSource.h"
#include "geometry/PolygonGeometry.h"
#include "renderers/drawdatas/PolygonDrawData.h"
#include "styles/PolygonStyle.h"

namespace carto {

    Polygon::Polygon(const std::shared_ptr<PolygonGeometry>& geometry, const std::shared_ptr<PolygonStyle>& style) :
        VectorElement(geometry),
        _style(style)
    {
    }
        
    Polygon::Polygon(const std::vector<MapPos>& poses, const std::shared_ptr<PolygonStyle>& style)  :
        VectorElement(std::make_shared<PolygonGeometry>(poses)),
        _style(style)
    {
    }
    
    Polygon::Polygon(const std::vector<MapPos>& poses, const std::vector<std::vector<MapPos> >& holes,
                     const std::shared_ptr<PolygonStyle>& style)  :
        VectorElement(std::make_shared<PolygonGeometry>(poses, holes)),
        _style(style)
    {
    }
    
    Polygon::~Polygon() {
    }
    
    std::shared_ptr<PolygonGeometry> Polygon::getGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::static_pointer_cast<PolygonGeometry>(_geometry);
    }
    
    void Polygon::setGeometry(const std::shared_ptr<PolygonGeometry>& geometry) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = geometry;
        }
        notifyElementChanged();
    }
    
    std::vector<MapPos> Polygon::getPoses() const {
        return getGeometry()->getPoses();
    }
        
    void Polygon::setPoses(const std::vector<MapPos>& poses) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = std::make_shared<PolygonGeometry>(poses, std::static_pointer_cast<PolygonGeometry>(_geometry)->getHoles());
        }
        notifyElementChanged();
    }

    std::vector<std::vector<MapPos> > Polygon::getHoles() const {
        return getGeometry()->getHoles();
    }
    
    void Polygon::setHoles(const std::vector<std::vector<MapPos> >& holes) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = std::make_shared<PolygonGeometry>(std::static_pointer_cast<PolygonGeometry>(_geometry)->getPoses(), holes);
        }
        notifyElementChanged();
    }
    
    std::shared_ptr<PolygonStyle> Polygon::getStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _style;
    }
    
    void Polygon::setStyle(const std::shared_ptr<PolygonStyle>& style) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<PolygonDrawData> Polygon::getDrawData() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _drawData;
    }
    
    void Polygon::setDrawData(const std::shared_ptr<PolygonDrawData>& drawData) {
        std::lock_guard<std::mutex> lock(_mutex);
        _drawData = drawData;
    }
    
}
