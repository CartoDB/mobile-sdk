#include "PopupDrawData.h"
#include "geometry/PointGeometry.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "styles/PopupStyle.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Popup.h"

namespace carto {

    PopupDrawData::PopupDrawData(Popup& popup, const PopupStyle& style, const Projection& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface, const std::shared_ptr<Options>& options, const ViewState& viewState) :
        BillboardDrawData(popup,
                          style,
                          projection,
                          projectionSurface,
                          popup.drawBitmap(CalculateAnchorScreenPos(popup, viewState, options, projection), viewState.getWidth(), viewState.getHeight(), viewState.getDPToPX()),
                          // Anchor point may be modified in Popup::drawBitmap, so order is important here
                          popup.getAnchorPointX(),
                          popup.getAnchorPointY(),
                          false,
                          BillboardOrientation::BILLBOARD_ORIENTATION_FACE_CAMERA,
                          BillboardScaling::BILLBOARD_SCALING_CONST_SCREEN_SIZE,
                          1.0f,
                          -1)
    {
        if (!style.isScaleWithDPI() || (style.isScaleWithDPI() && viewState.getDPI() >= Const::UNSCALED_DPI)) {
            // The generated texture will never be downscaled and thus doesn't need mipmaps
            _genMipmaps = false;
        }
    }
    
    PopupDrawData::~PopupDrawData() {
    }
    
    ScreenPos PopupDrawData::CalculateAnchorScreenPos(const Popup& popup, const ViewState& viewState, const std::shared_ptr<Options>& options, const Projection& projection) {
        if (std::shared_ptr<ProjectionSurface> projectionSurface = viewState.getProjectionSurface()) {
            cglib::vec2<float> screenPos = viewState.worldToScreen(projectionSurface->calculatePosition(projection.toInternal(popup.getRootGeometry()->getCenterPos())), options);
            return ScreenPos(screenPos(0), screenPos(1));
        }
        return ScreenPos();
    }
}
