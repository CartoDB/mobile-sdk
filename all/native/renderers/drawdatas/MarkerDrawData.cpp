#include "MarkerDrawData.h"
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
                          style.getSize())
    {
    }
    
    MarkerDrawData::~MarkerDrawData() {
    }
    
}
