#include "Billboard.h"
#include "components/Exceptions.h"
#include "geometry/PointGeometry.h"
#include "renderers/drawdatas/BillboardDrawData.h"
#include "utils/Log.h"

namespace carto {

    Billboard::~Billboard() {
    }
    
    std::shared_ptr<Billboard> Billboard::getBaseBillboard() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _baseBillboard;
    }
    
    void Billboard::setBaseBillboard(const std::shared_ptr<Billboard>& baseBillboard) {
        if (!baseBillboard) {
            throw NullArgumentException("Null baseBillboard");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (std::shared_ptr<Billboard> billboard = baseBillboard; billboard; billboard = billboard->getBaseBillboard()) {
                if (billboard.get() == this) {
                    throw InvalidArgumentException("Cycling loop when setting baseBillboard");
                }
            }
    
            _baseBillboard = baseBillboard;
            _geometry.reset();
        }
        notifyElementChanged();
    }
        
    MapBounds Billboard::getBounds() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::shared_ptr<Geometry> geometry = _geometry;
        if (!geometry) {
            std::shared_ptr<Billboard> baseBillboard = _baseBillboard;
            while (!geometry) {
                geometry = baseBillboard->getGeometry();
                baseBillboard = baseBillboard->getBaseBillboard();
            }
        }
        return geometry->getBounds();
    }
    
    std::shared_ptr<Geometry> Billboard::getRootGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::shared_ptr<Geometry> geometry = _geometry;
        if (!geometry) {
            std::shared_ptr<Billboard> baseBillboard = _baseBillboard;
            while (!geometry) {
                geometry = baseBillboard->getGeometry();
                baseBillboard = baseBillboard->getBaseBillboard();
            }
        }
        return geometry;
    }

    std::shared_ptr<Geometry> Billboard::getGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _geometry;
    }
    
    void Billboard::setGeometry(const std::shared_ptr<Geometry>& geometry) {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }
   
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = geometry;
            _baseBillboard.reset();
        }
        notifyElementChanged();
    }
        
    void Billboard::setPos(const MapPos& pos) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = std::make_shared<PointGeometry>(pos);
            _baseBillboard.reset();
        }
        notifyElementChanged();
    }
    
    float Billboard::getRotation() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _rotation;
    }
    
    void Billboard::setRotation(float rotation) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _rotation = rotation;
        }
        notifyElementChanged();
    }
        
    Billboard::Billboard(const std::shared_ptr<Billboard>& baseBillboard) :
        VectorElement(std::shared_ptr<Geometry>()),
        _baseBillboard(baseBillboard),
        _rotation(0)
    {
        if (!baseBillboard) {
            throw NullArgumentException("Null baseBillboard");
        }
    }
    
    Billboard::Billboard(const std::shared_ptr<Geometry>& geometry) :
        VectorElement(geometry),
        _baseBillboard(),
        _rotation(0)
    {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }
    }
        
    Billboard::Billboard(const MapPos& pos) :
        VectorElement(std::make_shared<PointGeometry>(pos)),
        _baseBillboard(),
        _rotation(0)
    {
    }
        
    std::shared_ptr<BillboardDrawData> Billboard::getDrawData() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _drawData;
    }
    
    void Billboard::setDrawData(const std::shared_ptr<BillboardDrawData>& drawData) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_drawData) {
            drawData->setOverlapping(_drawData->isOverlapping());
        }
        _drawData = drawData;
    }
        
}
