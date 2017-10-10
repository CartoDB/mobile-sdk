#include "MarkerDrawData.h"
#include "graphics/Bitmap.h"
#include "styles/MarkerStyle.h"
#include "vectorelements/Marker.h"

namespace carto {

    MarkerDrawData::MarkerDrawData(const Marker& marker, const MarkerStyle& style, const Projection& projection) :
        BillboardDrawData(marker,
                          style,
                          projection,
                          style.getBitmap(),
                          style.getAnchorPointX(),
                          style.getAnchorPointY(),
                          false,
                          style.getOrientationMode(),
                          style.getScalingMode(),
                          1.0f,
                          style.getSize()),
        _clickScale(1.0f)
    {
        if (style.getClickSize() != -1) {
            float size = style.getSize();
            if (size < 0) {
                size = (style.getBitmap() ? style.getBitmap()->getWidth() : 0);
            }
            _clickScale = style.getClickSize() / size;
        }
    }
    
    MarkerDrawData::~MarkerDrawData() {
    }

    float MarkerDrawData::getClickScale() const {
        return _clickScale;
    }
    
}
