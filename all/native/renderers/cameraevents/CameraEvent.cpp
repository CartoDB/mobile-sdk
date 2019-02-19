#include "CameraEvent.h"
#include "core/MapBounds.h"
#include "components/Options.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "utils/Const.h"
#include "utils/GeneralUtils.h"

namespace carto {

    void CameraEvent::ClampFocusPos(cglib::vec3<double>& focusPos, cglib::vec3<double>& cameraPos, cglib::vec3<double>& upVec, const Options& options, ViewState& viewState) {
        // Enforce map bounds
        std::shared_ptr<Projection> baseProjection = options.getBaseProjection();
        std::shared_ptr<ProjectionSurface> projectionSurface = options.getProjectionSurface();
        MapBounds mapBounds = options.getPanBounds();
        MapBounds projBounds = baseProjection->getBounds();
        MapPos mapPos = projectionSurface->calculateMapPos(focusPos);
        MapPos oldMapPos = mapPos;
        
        bool seamLess = options.isSeamlessPanning();
        RenderProjectionMode::RenderProjectionMode renderProjectionMode = options.getRenderProjectionMode();
        if ((!seamLess && renderProjectionMode == RenderProjectionMode::RENDER_PROJECTION_MODE_PLANAR) || mapBounds.getMin().getX() > projBounds.getMin().getX() || mapBounds.getMax().getX() < projBounds.getMax().getX()) {
            MapPos mapPosBase = baseProjection->fromInternal(mapPos);
            double x = GeneralUtils::Clamp(mapPosBase.getX(), mapBounds.getMin().getX(), mapBounds.getMax().getX());
            if (x != mapPosBase.getX()) {
                mapPos = baseProjection->toInternal(MapPos(x, mapPosBase.getY()));
            }
        }
        if (renderProjectionMode == RenderProjectionMode::RENDER_PROJECTION_MODE_PLANAR || mapBounds.getMin().getY() > projBounds.getMin().getY() || mapBounds.getMax().getY() < projBounds.getMax().getY()) {
            MapPos mapPosBase = baseProjection->fromInternal(mapPos);
            double y = GeneralUtils::Clamp(mapPosBase.getY(), mapBounds.getMin().getY(), mapBounds.getMax().getY());
            if (y != mapPosBase.getY()) {
                mapPos = baseProjection->toInternal(MapPos(mapPosBase.getX(), y));
            }
        }
        mapPos.setZ(0);

        // Teleport if necessary
        if (seamLess && renderProjectionMode == RenderProjectionMode::RENDER_PROJECTION_MODE_PLANAR) {
            if (mapPos.getX() > Const::HALF_WORLD_SIZE) {
                mapPos.setX(-Const::HALF_WORLD_SIZE + (mapPos.getX() - Const::HALF_WORLD_SIZE));
                viewState.setHorizontalLayerOffsetDir(-1);
            } else if (mapPos.getX() < -Const::HALF_WORLD_SIZE) {
                mapPos.setX(Const::HALF_WORLD_SIZE + (mapPos.getX() + Const::HALF_WORLD_SIZE));
                viewState.setHorizontalLayerOffsetDir(1);
            }
        }

        // Update camera parameters, if necessary
        if (mapPos != oldMapPos) {
            cglib::mat4x4<double> translateTransform = projectionSurface->calculateTranslateMatrix(focusPos, projectionSurface->calculatePosition(mapPos), 1);
            focusPos = cglib::transform_point(focusPos, translateTransform);
            cameraPos = cglib::transform_point(cameraPos, translateTransform);
            upVec = cglib::transform_vector(upVec, translateTransform);
        }
    }
    
}
