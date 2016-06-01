#include "NMLModel.h"
#include "core/BinaryData.h"
#include "geometry/PointGeometry.h"
#include "renderers/drawdatas/NMLModelDrawData.h"
#include "utils/Const.h"
#include "nml/nmlpackage/NMLPackage.pb.h"

namespace carto {

    NMLModel::NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<nml::Model>& sourceModel) :
        VectorElement(geometry),
        _rotationAxis(0, 0, 1),
        _rotationAngle(0),
        _scale(1),
        _sourceModel(sourceModel)
    {
        const nml::Vector3& minBounds = _sourceModel->bounds().min();
        const nml::Vector3& maxBounds = _sourceModel->bounds().max();
        _bounds = MapBounds(MapPos(minBounds.x(), minBounds.y(), minBounds.z()), MapPos(maxBounds.x(), maxBounds.y(), maxBounds.z()));
    }
    
    NMLModel::NMLModel(const MapPos& mapPos, const std::shared_ptr<nml::Model>& sourceModel) :
        VectorElement(std::make_shared<PointGeometry>(mapPos)),
        _rotationAxis(0, 0, 1),
        _rotationAngle(0),
        _scale(1),
        _sourceModel(sourceModel)
    {
        const nml::Vector3& minBounds = _sourceModel->bounds().min();
        const nml::Vector3& maxBounds = _sourceModel->bounds().max();
        _bounds = MapBounds(MapPos(minBounds.x(), minBounds.y(), minBounds.z()), MapPos(maxBounds.x(), maxBounds.y(), maxBounds.z()));
    }
    
    NMLModel::NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<BinaryData>& sourceModelData) :
        VectorElement(geometry),
        _rotationAxis(0, 0, 1),
        _rotationAngle(0),
        _scale(1),
        _sourceModel()
    {
        std::shared_ptr<std::vector<unsigned char> > data = sourceModelData->getDataPtr();
        protobuf::message modelMsg(data->data(), data->size());
        _sourceModel = std::make_shared<nml::Model>(modelMsg);
        const nml::Vector3& minBounds = _sourceModel->bounds().min();
        const nml::Vector3& maxBounds = _sourceModel->bounds().max();
        _bounds = MapBounds(MapPos(minBounds.x(), minBounds.y(), minBounds.z()), MapPos(maxBounds.x(), maxBounds.y(), maxBounds.z()));
    }
    
    NMLModel::NMLModel(const MapPos& mapPos, const std::shared_ptr<BinaryData>& sourceModelData) :
        VectorElement(std::make_shared<PointGeometry>(mapPos)),
        _rotationAxis(0, 0, 1),
        _rotationAngle(0),
        _scale(1),
        _sourceModel()
    {
        std::shared_ptr<std::vector<unsigned char> > data = sourceModelData->getDataPtr();
        protobuf::message modelMsg(data->data(), data->size());
        _sourceModel = std::make_shared<nml::Model>(modelMsg);
        const nml::Vector3& minBounds = _sourceModel->bounds().min();
        const nml::Vector3& maxBounds = _sourceModel->bounds().max();
        _bounds = MapBounds(MapPos(minBounds.x(), minBounds.y(), minBounds.z()), MapPos(maxBounds.x(), maxBounds.y(), maxBounds.z()));
    }
    
    NMLModel::~NMLModel() {
    }
        
    MapBounds NMLModel::getBounds() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bounds;
    }
    
    void NMLModel::setGeometry(const std::shared_ptr<Geometry>& geometry) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = geometry;
        }
        notifyElementChanged();
    }
    
    void NMLModel::setPos(const MapPos& pos) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = std::make_shared<PointGeometry>(pos);
        }
        notifyElementChanged();
    }
        
    cglib::mat4x4<float> NMLModel::getLocalMat() const {
        std::lock_guard<std::mutex> lock(_mutex);
        cglib::mat4x4<float> rotateMat = cglib::rotate4_matrix(cglib::vec3<float>((float) _rotationAxis.getX(), (float) _rotationAxis.getY(), (float) _rotationAxis.getZ()), _rotationAngle * static_cast<float>(Const::DEG_TO_RAD));
        cglib::mat4x4<float> scaleMat = cglib::scale4_matrix(cglib::vec3<float>(_scale, _scale, _scale));
        return rotateMat * scaleMat;
    }
    
    float NMLModel::getRotationAngle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _rotationAngle;
    }
    
    MapVec NMLModel::getRotationAxis() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _rotationAxis;
    }
    
    void NMLModel::setRotation(const MapVec& axis, float angle) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _rotationAxis = axis;
            _rotationAngle = angle;
        }
        notifyElementChanged();
    }
    
    float NMLModel::getScale() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _scale;
    }
    
    void NMLModel::setScale(float scale) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _scale = scale;
        }
        notifyElementChanged();
    }
        
    std::shared_ptr<nml::Model> NMLModel::getSourceModel() const {
        return _sourceModel;
    }
    
    std::shared_ptr<NMLModelDrawData> NMLModel::getDrawData() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _drawData;
    }
    
    void NMLModel::setDrawData(const std::shared_ptr<NMLModelDrawData>& drawData) {
        std::lock_guard<std::mutex> lock(_mutex);
        _drawData = drawData;
    }
    
}
