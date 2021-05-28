#include "NMLModelDrawData.h"
#include "geometry/Geometry.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "styles/NMLModelStyle.h"
#include "vectorelements/NMLModel.h"
#include "utils/Const.h"

#include <nml/Package.h>

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

        // Calculate local transformation matrix
        MapVec rotationAxis = model.getRotationAxis();
        cglib::mat4x4<double> rotateMat = cglib::rotate4_matrix(cglib::vec3<double>(rotationAxis.getX(), rotationAxis.getY(), rotationAxis.getZ()), model.getRotationAngle() * Const::DEG_TO_RAD);
        cglib::mat4x4<double> scaleMat = cglib::scale4_matrix(cglib::vec3<double>(model.getScale(), model.getScale(), model.getScale()));
        _localTransformMat = rotateMat * scaleMat;
        
        // Calculate local frame matrix. We will reset the axis here, as orientation will be handled separately.
        _localFrameMat = projectionSurface->calculateLocalFrameMatrix(projectionSurface->calculatePosition(mapPosInternal));
        for (int i = 0; i < 3; i++) {
            cglib::vec3<double> axis;
            for (int j = 0; j < 3; j++) {
                axis(j) = _localFrameMat(i, j);
            }
            float c = cglib::length(axis);
            for (int j = 0; j < 3; j++) {
                _localFrameMat(i, j) = (i == j ? c : 0);
            }
        }

        // Calculate bounding box coordinates
        const nml::Vector3& minBounds = _sourceModel->bounds().min();
        const nml::Vector3& maxBounds = _sourceModel->bounds().max();
        _sourceModelBounds = cglib::bbox3<float>(cglib::vec3<float>(minBounds.x(), minBounds.y(), minBounds.z()), cglib::vec3<float>(maxBounds.x(), maxBounds.y(), maxBounds.z()));
    }

    NMLModelDrawData::~NMLModelDrawData() {
    }
    
    std::shared_ptr<nml::Model> NMLModelDrawData::getSourceModel() const {
        return _sourceModel;
    }

    const cglib::bbox3<float>& NMLModelDrawData::getSourceModelBounds() const {
        return _sourceModelBounds;
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
