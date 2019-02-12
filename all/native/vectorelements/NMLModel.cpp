#include "NMLModel.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geometry/PointGeometry.h"
#include "renderers/drawdatas/NMLModelDrawData.h"
#include "utils/Const.h"

#include <nml/Package.h>

namespace carto {

    NMLModel::NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<nml::Model>& sourceModel) :
        VectorElement(geometry),
        _rotationAxis(0, 0, 1),
        _rotationAngle(0),
        _scale(1),
        _sourceModel(sourceModel)
    {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }
        if (!sourceModel) {
            throw NullArgumentException("Null sourceModel");
        }
    }
    
    NMLModel::NMLModel(const MapPos& mapPos, const std::shared_ptr<nml::Model>& sourceModel) :
        VectorElement(std::make_shared<PointGeometry>(mapPos)),
        _rotationAxis(0, 0, 1),
        _rotationAngle(0),
        _scale(1),
        _sourceModel(sourceModel)
    {
        if (!sourceModel) {
            throw NullArgumentException("Null sourceModel");
        }
    }
    
    NMLModel::NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<BinaryData>& sourceModelData) :
        VectorElement(geometry),
        _rotationAxis(0, 0, 1),
        _rotationAngle(0),
        _scale(1),
        _sourceModel()
    {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }
        if (!sourceModelData) {
            throw NullArgumentException("Null sourceModelData");
        }

        std::shared_ptr<std::vector<unsigned char> > data = sourceModelData->getDataPtr();
        protobuf::message modelMsg(data->data(), data->size());
        _sourceModel = std::make_shared<nml::Model>(modelMsg);
    }
    
    NMLModel::NMLModel(const MapPos& mapPos, const std::shared_ptr<BinaryData>& sourceModelData) :
        VectorElement(std::make_shared<PointGeometry>(mapPos)),
        _rotationAxis(0, 0, 1),
        _rotationAngle(0),
        _scale(1),
        _sourceModel()
    {
        if (!sourceModelData) {
            throw NullArgumentException("Null sourceModelData");
        }

        std::shared_ptr<std::vector<unsigned char> > data = sourceModelData->getDataPtr();
        protobuf::message modelMsg(data->data(), data->size());
        _sourceModel = std::make_shared<nml::Model>(modelMsg);
    }
    
    NMLModel::~NMLModel() {
    }
        
    void NMLModel::setGeometry(const std::shared_ptr<Geometry>& geometry) {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _geometry = geometry;
        }
        notifyElementChanged();
    }
    
    void NMLModel::setPos(const MapPos& pos) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _geometry = std::make_shared<PointGeometry>(pos);
        }
        notifyElementChanged();
    }
        
    float NMLModel::getRotationAngle() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _rotationAngle;
    }
    
    MapVec NMLModel::getRotationAxis() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _rotationAxis;
    }
    
    void NMLModel::setRotation(const MapVec& axis, float angle) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _rotationAxis = axis;
            _rotationAngle = angle;
        }
        notifyElementChanged();
    }
    
    float NMLModel::getScale() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _scale;
    }
    
    void NMLModel::setScale(float scale) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _scale = scale;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<nml::Model> NMLModel::getSourceModel() const {
        return _sourceModel;
    }
    
    std::shared_ptr<NMLModelDrawData> NMLModel::getDrawData() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _drawData;
    }
    
    void NMLModel::setDrawData(const std::shared_ptr<NMLModelDrawData>& drawData) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _drawData = drawData;
    }
    
}
