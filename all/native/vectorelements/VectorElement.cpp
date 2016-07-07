#include "VectorElement.h"
#include "datasources/VectorDataSource.h"
#include "geometry/Geometry.h"
#include "layers/Layer.h"
#include "utils/Log.h"

namespace carto {

    VectorElement::~VectorElement() {
    }
    
    MapBounds VectorElement::getBounds() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _geometry->getBounds();
    }
    
    std::shared_ptr<Geometry> VectorElement::getGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _geometry;
    }
    
    long long VectorElement::getId() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _id;
    }
    
    void VectorElement::setId(long long id) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _id = id;
        }
        notifyElementChanged();
    }
    
    std::map<std::string, Variant> VectorElement::getMetaData() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _metaData;
    }
        
    void VectorElement::setMetaData(const std::map<std::string, Variant>& metaData) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _metaData = metaData;
        }
        notifyElementChanged();
    }
    
    Variant VectorElement::getMetaDataElement(const std::string& key) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _metaData.find(key);
        if (it == _metaData.end()) {
            return Variant();
        }
        return it->second;
    }
    
    void VectorElement::setMetaDataElement(const std::string& key, const Variant& element) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _metaData[key] = element;
        }
        notifyElementChanged();
    }
    
    bool VectorElement::isVisible() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _visible;
    }
    
    void VectorElement::setVisible(bool visible) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _visible = visible;
        }
        notifyElementChanged();
    }
    
    void VectorElement::notifyElementChanged() {
        std::shared_ptr<VectorDataSource> dataSource;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            dataSource = _dataSource.lock();;
        }
        if (dataSource) {
            dataSource->notifyElementChanged(shared_from_this());
        }
    }
    
    VectorElement::VectorElement(const std::shared_ptr<Geometry>& geometry) :
        _dataSource(),
        _geometry(geometry),
        _mutex(),
        _id(-1),
        _metaData(),
        _visible(true)
    {
        if (!geometry) {
            throw std::invalid_argument("Null geometry");
        }
    }
        
    void VectorElement::attachToDataSource(const std::weak_ptr<VectorDataSource>& dataSource) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_dataSource.lock() && dataSource.lock()) {
            Log::Error("VectorElement::attachToDataSource: Vector element is already attached to a data source");
            return;
        }
        _dataSource = dataSource;
    }
    
    void VectorElement::detachFromDataSource() {
        std::lock_guard<std::mutex> lock(_mutex);
        _dataSource.reset();
    }
    
}
