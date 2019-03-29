#include "BillboardSorter.h"
#include "graphics/ViewState.h"
#include "renderers/drawdatas/BillboardDrawData.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Billboard.h"

#include <functional>

namespace carto {

    BillboardSorter::BillboardSorter() :
        _billboardDrawDatas()
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
    
        // Calculate billboard distances
        const cglib::mat4x4<double>& mvpMat = viewState.getModelviewProjectionMat();
        for (const std::shared_ptr<BillboardDrawData>& drawData : _billboardDrawDatas) {
            const cglib::vec3<double>& pos = drawData->getPos();
    
            // Calculate distance to the camera plane, adjust to zoom
            double distance = pos(0) * mvpMat(3, 0) + pos(1) * mvpMat(3, 1) + pos(2) * mvpMat(3, 2) + mvpMat(3, 3);
            double zoomDistance = distance * viewState.get2PowZoom() / viewState.getZoom0Distance();
            drawData->setCameraPlaneZoomDistance(zoomDistance);
    
            // Calculate distance from the bottom of the screen
            cglib::vec2<float> screenPos = viewState.worldToScreen(pos);
            drawData->setScreenBottomDistance(viewState.getHeight() - screenPos(1));
        }

        // Sort billboards
        auto distanceComparator = [](const std::shared_ptr<BillboardDrawData>& drawData1, const std::shared_ptr<BillboardDrawData>& drawData2) {
            // First compare placement priorities
            float priorityDelta = drawData2->getPlacementPriority() - drawData1->getPlacementPriority();
            if (priorityDelta != 0) {
                return priorityDelta > 0;
            }
    
            // Sort by the distance to the camera plane
            double cameraPlaneZoomDistDelta = drawData2->getCameraPlaneZoomDistance() - drawData1->getCameraPlaneZoomDistance();
            if (cameraPlaneZoomDistDelta != 0) {
                return cameraPlaneZoomDistDelta < 0;
            }

            // Sort by the distance to the bottom of screen
            return drawData1->getScreenBottomDistance() > drawData2->getScreenBottomDistance();
        };
        std::stable_sort(_billboardDrawDatas.begin(), _billboardDrawDatas.end(), distanceComparator);
    }
    
    const std::vector<std::shared_ptr<BillboardDrawData> >& BillboardSorter::getSortedBillboardDrawDatas() const {
        return _billboardDrawDatas;
    }
    
}
