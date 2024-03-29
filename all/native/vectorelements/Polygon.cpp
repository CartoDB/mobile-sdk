#include "Polygon.h"
#include "components/Exceptions.h"
#include "datasources/VectorDataSource.h"
#include "geometry/PolygonGeometry.h"
#include "renderers/drawdatas/PolygonDrawData.h"
#include "styles/PolygonStyle.h"

namespace carto {

    Polygon::Polygon(const std::shared_ptr<PolygonGeometry>& geometry, const std::shared_ptr<PolygonStyle>& style) :
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
        
    Polygon::Polygon(std::vector<MapPos> poses, const std::shared_ptr<PolygonStyle>& style)  :
        VectorElement(std::make_shared<PolygonGeometry>(std::move(poses))),
        _drawData(),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
    
    Polygon::Polygon(std::vector<MapPos> poses, std::vector<std::vector<MapPos> > holes,
                     const std::shared_ptr<PolygonStyle>& style)  :
        VectorElement(std::make_shared<PolygonGeometry>(std::move(poses), std::move(holes))),
        _drawData(),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
    
    Polygon::~Polygon() {
    }
    
    std::shared_ptr<PolygonGeometry> Polygon::getGeometry() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return std::static_pointer_cast<PolygonGeometry>(_geometry);
    }
    
    void Polygon::setGeometry(const std::shared_ptr<PolygonGeometry>& geometry) {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _geometry = geometry;
        }
        notifyElementChanged();
    }
    
    std::vector<MapPos> Polygon::getPoses() const {
        return getGeometry()->getPoses();
    }
        
    void Polygon::setPoses(const std::vector<MapPos>& poses) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _geometry = std::make_shared<PolygonGeometry>(poses, std::static_pointer_cast<PolygonGeometry>(_geometry)->getHoles());
        }
        notifyElementChanged();
    }

    std::vector<std::vector<MapPos> > Polygon::getHoles() const {
        return getGeometry()->getHoles();
    }
    
    void Polygon::setHoles(const std::vector<std::vector<MapPos> >& holes) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _geometry = std::make_shared<PolygonGeometry>(std::static_pointer_cast<PolygonGeometry>(_geometry)->getPoses(), holes);
        }
        notifyElementChanged();
    }
    
    std::shared_ptr<PolygonStyle> Polygon::getStyle() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _style;
    }
    
    void Polygon::setStyle(const std::shared_ptr<PolygonStyle>& style) {
        if (!style) {
            throw NullArgumentException("Null style");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<PolygonDrawData> Polygon::getDrawData() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _drawData;
    }
    
    void Polygon::setDrawData(const std::shared_ptr<PolygonDrawData>& drawData) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _drawData = drawData;
    }
    
}
