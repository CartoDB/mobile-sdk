#include "PopupDrawData.h"
#include "geometry/PointGeometry.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "styles/PopupStyle.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Popup.h"

namespace carto {

    PopupDrawData::PopupDrawData(Popup& popup, const PopupStyle& style, const Projection& projection, const Options& options,
                                 const ViewState& viewState) :
        BillboardDrawData(popup,
                          style,
                          projection,
                          popup.drawBitmap(viewState.worldToScreen(projection.toInternal(popup.getRootGeometry()->getCenterPos()), options),
                                           viewState.getWidth(),
                                           viewState.getHeight(),
                                           viewState.getDPToPX()),
                          // Anchor point may be modified in Popup::drawBitmap, so order is important here
                          popup.getAnchorPointX(),
                          popup.getAnchorPointY(),
                          false,
                          BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA,
                          BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE,
                          -1)
    {
        if (!style.isScaleWithDPI() || (style.isScaleWithDPI() && viewState.getDPI() >= Const::UNSCALED_DPI)) {
            //The generated texture will never be downscaled and thus doesn't need mipmaps
            _genMipmaps = false;
        }
    }
    
    PopupDrawData::~PopupDrawData() {
    }
    
}
