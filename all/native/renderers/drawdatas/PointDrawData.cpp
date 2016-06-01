#include "PointDrawData.h"
#include "graphics/Bitmap.h"
#include "geometry/PointGeometry.h"
#include "projections/Projection.h"
#include "styles/PointStyle.h"
#include "vectorelements/Point.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <utility>
#include <algorithm>

namespace carto {

    PointDrawData::PointDrawData(const PointGeometry& geometry, const PointStyle& style, const Projection& projection) :
        VectorElementDrawData(style.getColor()),
        _bitmap(style.getBitmap()),
        _clickScale(style.getClickSize() == -1 ? std::max(1.0f, 1 + (IDEAL_CLICK_SIZE - style.getSize()) * CLICK_SIZE_COEF / style.getSize()) : style.getClickSize()),
        _pos(projection.toInternal(geometry.getPos())),
        _size(style.getSize())
    {
    }
    
    PointDrawData::~PointDrawData() {
    }
    
    const std::shared_ptr<Bitmap> PointDrawData::getBitmap() const {
        return _bitmap;
    }
    
    float PointDrawData::getClickScale() const {
        return _clickScale;
    }
    
    const MapPos& PointDrawData::getPos() const {
        return _pos;
    }
    
    float PointDrawData::getSize() const {
        return _size;
    }
    
    void PointDrawData::offsetHorizontally(double offset) {
        _pos.setX(_pos.getX() + offset);
        setIsOffset(true);
    }
    
    const float PointDrawData::CLICK_SIZE_COEF = 0.5f;
    
}
