#include "LabelDrawData.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "styles/LabelStyle.h"
#include "utils/Const.h"
#include "vectorelements/Label.h"

namespace carto {

    LabelDrawData::LabelDrawData(const Label& label, const LabelStyle& style, const Projection& projection, const ProjectionSurface& projectionSurface, const ViewState& viewState) :
        BillboardDrawData(label,
                          style,
                          projection,
                          projectionSurface,
                          label.drawBitmap(viewState.getDPToPX()),
                          style.getAnchorPointX(),
                          style.getAnchorPointY(),
                          style.isFlippable(),
                          style.getOrientationMode(),
                          style.getScalingMode(),
                          style.getRenderScale(),
                          -1)
    {
        if (style.getOrientationMode() == BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA &&
            style.getScalingMode() == BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE &&
            style.getRenderScale() == 1.0f &&
            (!style.isScaleWithDPI() || (style.isScaleWithDPI() && viewState.getDPI() >= Const::UNSCALED_DPI))) {
            // The generated texture will never be downscaled and thus doesn't need mipmaps
            _genMipmaps = false;
        }
    }
    
    LabelDrawData::~LabelDrawData() {
    }

}
