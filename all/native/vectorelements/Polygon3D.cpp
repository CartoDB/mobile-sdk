#include "Polygon3D.h"
#include "datasources/VectorDataSource.h"
#include "geometry/PolygonGeometry.h"
#include "renderers/drawdatas/Polygon3DDrawData.h"

namespace carto {

    Polygon3D::Polygon3D(const std::shared_ptr<PolygonGeometry>& geometry, const std::shared_ptr<Polygon3DStyle>& style, float height) :
        VectorElement(geometry),
        _height(height),
        _style(style)
    {
        if (!geometry) {
            throw std::invalid_argument("Null geometry");
        }
        if (!style) {
            throw std::invalid_argument("Null style");
        }
    }
        
    Polygon3D::Polygon3D(const std::vector<MapPos>& poses, const std::shared_ptr<Polygon3DStyle>& style, float height) :
        VectorElement(std::make_shared<PolygonGeometry>(poses)),
        _height(height),
        _style(style)
    {
        if (!style) {
            throw std::invalid_argument("Null style");
        }
    }
    
    Polygon3D::Polygon3D(const std::vector<MapPos>& poses, const std::vector<std::vector<MapPos> >& holes,
                     const std::shared_ptr<Polygon3DStyle>& style, float height) :
        VectorElement(std::make_shared<PolygonGeometry>(poses, holes)),
        _height(height),
        _style(style)
    {
        if (!style) {
            throw std::invalid_argument("Null style");
        }
    }
    
    Polygon3D::~Polygon3D() {
    }
    
    std::shared_ptr<PolygonGeometry> Polygon3D::getGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::static_pointer_cast<PolygonGeometry>(_geometry);
    }
    
    void Polygon3D::setGeometry(const std::shared_ptr<PolygonGeometry>& geometry) {
        if (!geometry) {
            throw std::invalid_argument("Null geometry");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = geometry;
        }
        notifyElementChanged();
    }
    
    std::vector<MapPos> Polygon3D::getPoses() const {
        return getGeometry()->getPoses();
    }
        
    void Polygon3D::setPoses(const std::vector<MapPos>& poses) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = std::make_shared<PolygonGeometry>(poses, std::static_pointer_cast<PolygonGeometry>(_geometry)->getHoles());
        }
        notifyElementChanged();
    }
    
    std::vector<std::vector<MapPos> > Polygon3D::getHoles() const {
        return getGeometry()->getHoles();
    }
    
    void Polygon3D::setHoles(const std::vector<std::vector<MapPos> >& holes) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = std::make_shared<PolygonGeometry>(std::static_pointer_cast<PolygonGeometry>(_geometry)->getPoses(), holes);
        }
        notifyElementChanged();
    }
    
    float Polygon3D::getHeight() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _height;
    }
    
    void Polygon3D::setHeight(float height) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _height = height;
        }
        notifyElementChanged();
    }
    
    std::shared_ptr<Polygon3DStyle> Polygon3D::getStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _style;
    }
    
    void Polygon3D::setStyle(const std::shared_ptr<Polygon3DStyle>& style) {
        if (!style) {
            throw std::invalid_argument("Null style");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<Polygon3DDrawData> Polygon3D::getDrawData() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _drawData;
    }
    
    void Polygon3D::setDrawData(const std::shared_ptr<Polygon3DDrawData>& drawData) {
        std::lock_guard<std::mutex> lock(_mutex);
        _drawData = drawData;
    }
    
}
