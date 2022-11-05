#include "NMLModel.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geometry/PointGeometry.h"
#include "renderers/drawdatas/NMLModelDrawData.h"
#include "styles/NMLModelStyle.h"
#include "styles/NMLModelStyleBuilder.h"
#include "utils/Const.h"

#include <nml/Package.h>

namespace carto {

    NMLModel::NMLModel(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<NMLModelStyle>& style) :
        Billboard(baseBillboard),
        _rotationAxis(0, 0, 1),
        _scale(1),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }

    NMLModel::NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<NMLModelStyle>& style) :
        Billboard(geometry),
        _rotationAxis(0, 0, 1),
        _scale(1),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
    
    NMLModel::NMLModel(const MapPos& mapPos, const std::shared_ptr<NMLModelStyle>& style) :
        Billboard(mapPos),
        _rotationAxis(0, 0, 1),
        _scale(1),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
    
    NMLModel::~NMLModel() {
    }
        
    float NMLModel::getRotationAngle() const {
        return Billboard::getRotation();
    }
    
    void NMLModel::setRotationAngle(float angle) {
        Billboard::setRotation(angle);
    }
    
    MapVec NMLModel::getRotationAxis() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _rotationAxis;
    }
    
    void NMLModel::setRotationAxis(const MapVec& axis) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _rotationAxis = axis;
        }
        notifyElementChanged();
    }
    
    void NMLModel::setRotation(const MapVec& axis, float angle) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _rotationAxis = axis;
            _rotation = angle;
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
        
    std::shared_ptr<NMLModelStyle> NMLModel::getStyle() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _style;
    }

    void NMLModel::setStyle(const std::shared_ptr<NMLModelStyle>& style) {
        if (!style) {
            throw NullArgumentException("Null style");
        }

        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }
    
}
