#include "CameraEvent.h"
#include "core/MapBounds.h"
#include "components/Options.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "utils/Const.h"
#include "utils/GeneralUtils.h"

namespace carto {

    void CameraEvent::ClampFocusPos(cglib::vec3<double>& focusPos, const Options& options) {
        // TODO: implement
        /*
        // Enforce map bounds
        std::shared_ptr<Projection> baseProjection = options.getBaseProjection();
        MapBounds projBounds = baseProjection->getBounds();
        MapBounds mapBounds = options.getPanBounds();
        bool seamLess = options.isSeamlessPanning();
        if (!seamLess || mapBounds.getMin().getX() > projBounds.getMin().getX() || mapBounds.getMax().getX() < projBounds.getMax().getX()) {
            focusPos.setX(GeneralUtils::Clamp(focusPos.getX(), mapBounds.getMin().getX(), mapBounds.getMax().getX()));
        }
        focusPos.setY(GeneralUtils::Clamp(focusPos.getY(), mapBounds.getMin().getY(), mapBounds.getMax().getY()));
        */

        // TODO: remove altogether?
        /*
        // Teleport if necessary
        if (seamLess) {
            if (focusPos.getX() > Const::HALF_WORLD_SIZE) {
                focusPos.setX(-Const::HALF_WORLD_SIZE + (focusPos.getX() - Const::HALF_WORLD_SIZE));
                viewState.setHorizontalLayerOffsetDir(-1);
            } else if (focusPos.getX() < -Const::HALF_WORLD_SIZE) {
                focusPos.setX(Const::HALF_WORLD_SIZE + (focusPos.getX() + Const::HALF_WORLD_SIZE));
                viewState.setHorizontalLayerOffsetDir(1);
            }
        }
        */
    }
    
}
