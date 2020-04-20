#include "BillboardSorter.h"
#include "graphics/ViewState.h"
#include "projections/PlanarProjectionSurface.h"
#include "renderers/drawdatas/BillboardDrawData.h"
#include "renderers/drawdatas/PopupDrawData.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Billboard.h"

#include <functional>

namespace carto {

    BillboardSorter::BillboardSorter(std::vector<std::shared_ptr<BillboardDrawData> >& billboardDrawDatas) :
        _billboardDrawDatas(billboardDrawDatas)
    {
    }
    
    BillboardSorter::~BillboardSorter() {
    
    }
    
    void BillboardSorter::clear() {
        // Resize, but don't reallocate
        _billboardDrawDatas.clear();
    }
    
    void BillboardSorter::add(const std::shared_ptr<BillboardDrawData>& drawData) {
        _billboardDrawDatas.push_back(drawData);
    }
    
    void BillboardSorter::sort(const ViewState& viewState) {
        if (_billboardDrawDatas.empty()) {
            return;
        }

        // Special '2D' mode
        bool is2DMode = false;
        if (viewState.getTilt() == 90) {
            if (viewState.getZoom() >= PLANAR_ZOOM_THRESHOLD || std::dynamic_pointer_cast<PlanarProjectionSurface>(viewState.getProjectionSurface())) {
                is2DMode = true;
            }
        }
    
        // Calculate billboard distances
        const cglib::mat4x4<double>& mvpMat = viewState.getModelviewProjectionMat();
        for (const std::shared_ptr<BillboardDrawData>& drawData : _billboardDrawDatas) {
            const cglib::vec3<double>& pos = drawData->getPos();

            // If in 2D mode, calculate proper distance from the bottom of the screen.
            if (is2DMode) {
                cglib::vec2<float> screenPos = viewState.worldToScreen(pos);
                drawData->setScreenBottomDistance(viewState.getHeight() - std::floor(screenPos(1)));
            } else {
                drawData->setScreenBottomDistance(viewState.getHeight());
            }
    
            // Calculate distance to the camera plane, adjust to zoom
            double distance = pos(0) * mvpMat(3, 0) + pos(1) * mvpMat(3, 1) + pos(2) * mvpMat(3, 2) + mvpMat(3, 3);
            double zoomDistance = distance * viewState.get2PowZoom() / viewState.getZoom0Distance();
            drawData->setCameraPlaneZoomDistance(zoomDistance);
        }

        // Sort billboards
        auto distanceComparator = [](const std::shared_ptr<BillboardDrawData>& drawData1, const std::shared_ptr<BillboardDrawData>& drawData2) {
            return drawData1->isBefore(*drawData2);
        };
        std::stable_sort(_billboardDrawDatas.begin(), _billboardDrawDatas.end(), distanceComparator);
    }
    
    const float BillboardSorter::PLANAR_ZOOM_THRESHOLD = 10.0f;

}
