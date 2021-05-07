#include "NMLModelDrawData.h"
#include "geometry/Geometry.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "styles/NMLModelStyle.h"
#include "vectorelements/NMLModel.h"
#include "utils/Const.h"

namespace carto {

    NMLModelDrawData::NMLModelDrawData(const NMLModel& model, const NMLModelStyle& style, const Projection& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface) :
        BillboardDrawData(model,
                          style,
                          projection,
                          projectionSurface,
                          std::shared_ptr<Bitmap>(),
                          0.0f,
                          0.0f,
                          true,
                          style.getOrientationMode(),
                          style.getScalingMode(),
                          1.0f,
                          1.0f),
        _sourceModel(style.getSourceModel()),
        _localFrameMat(),
        _localTransformMat()
    {
        MapPos mapPosInternal = projection.toInternal(model.getGeometry()->getCenterPos());
        cglib::mat4x4<double> rotateMat = cglib::rotate4_matrix(projectionSurface->calculateVector(mapPosInternal, model.getRotationAxis()), model.getRotationAngle() * Const::DEG_TO_RAD);
        cglib::mat4x4<double> scaleMat = cglib::scale4_matrix(cglib::vec3<double>(model.getScale(), model.getScale(), model.getScale()));
        _localFrameMat = projectionSurface->calculateLocalFrameMatrix(projectionSurface->calculatePosition(mapPosInternal));
        _localTransformMat = rotateMat * scaleMat;
    }

    NMLModelDrawData::~NMLModelDrawData() {
    }
    
    std::shared_ptr<nml::Model> NMLModelDrawData::getSourceModel() const {
        return _sourceModel;
    }
    
    const cglib::mat4x4<double>& NMLModelDrawData::getLocalFrameMat() const {
        return _localFrameMat;
    }
    
    const cglib::mat4x4<double>& NMLModelDrawData::getLocalTransformMat() const {
        return _localTransformMat;
    }
    
    void NMLModelDrawData::offsetHorizontally(double offset) {
        _localFrameMat = cglib::translate4_matrix(cglib::vec3<double>(offset, 0, 0)) * _localFrameMat;
        setIsOffset(true);
    }

}
