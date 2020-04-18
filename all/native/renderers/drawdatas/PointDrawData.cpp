#include "PointDrawData.h"
#include "graphics/Bitmap.h"
#include "geometry/PointGeometry.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "styles/PointStyle.h"
#include "vectorelements/Point.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <utility>
#include <algorithm>

namespace carto {

    PointDrawData::PointDrawData(const PointGeometry& geometry, const PointStyle& style, const Projection& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface) :
        VectorElementDrawData(style.getColor(), projectionSurface),
        _bitmap(style.getBitmap()),
        _clickScale(style.getClickSize() == -1 ? std::max(1.0f, 1 + (IDEAL_CLICK_SIZE - style.getSize()) * CLICK_SIZE_COEF / style.getSize()) : style.getClickSize() / style.getSize()),
        _pos(),
        _xAxis(),
        _yAxis(),
        _size(style.getSize())
    {
        MapPos internalPos = projection.toInternal(geometry.getCenterPos());
        _pos = projectionSurface->calculatePosition(internalPos);
        _xAxis = cglib::vec3<float>::convert(cglib::unit(projectionSurface->calculateVector(internalPos, MapVec(1, 0, 0))));
        _yAxis = cglib::vec3<float>::convert(cglib::unit(projectionSurface->calculateVector(internalPos, MapVec(0, 1, 0))));
    }
    
    PointDrawData::~PointDrawData() {
    }
    
    const std::shared_ptr<Bitmap> PointDrawData::getBitmap() const {
        return _bitmap;
    }
    
    float PointDrawData::getClickScale() const {
        return _clickScale;
    }
    
    const cglib::vec3<double>& PointDrawData::getPos() const {
        return _pos;
    }
    
    const cglib::vec3<float>& PointDrawData::getXAxis() const {
        return _xAxis;
    }

    const cglib::vec3<float>& PointDrawData::getYAxis() const {
        return _yAxis;
    }

    float PointDrawData::getSize() const {
        return _size;
    }
    
    void PointDrawData::offsetHorizontally(double offset) {
        _pos(0) += offset;
        setIsOffset(true);
    }
    
    const float PointDrawData::CLICK_SIZE_COEF = 0.5f;
    
}
