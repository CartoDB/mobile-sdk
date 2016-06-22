#include "BillboardSorter.h"
#include "graphics/ViewState.h"
#include "renderers/drawdatas/BillboardDrawData.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"
#include "vectorelements/Billboard.h"

#include <functional>

namespace carto {

    BillboardSorter::BillboardSorter() :
        _billboardDrawDatas(),
        _sort3D(true)
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
    
        // Calculate the world positions of the bottom screen corners, offset them by a little to avoid certain artifacts
        MapPos bottomLeft(viewState.screenToWorldPlane(ScreenPos(0, viewState.getHeight() * 1.5f), 0));
        MapPos bottomRight(viewState.screenToWorldPlane(ScreenPos(viewState.getWidth(), viewState.getHeight() * 1.5f), 0));
        _sort3D = viewState.getTilt() < 90;
    
        // Calculate billboard distances
        const cglib::mat4x4<double>& mvpMat = viewState.getModelviewProjectionMat();
        for (const std::shared_ptr<BillboardDrawData>& drawData : _billboardDrawDatas) {
            const cglib::vec3<double>& pos = drawData->getPos();
    
            // Calculate distance to the camera plane, adjust to zoom
            double distance = pos(0) * mvpMat(3, 0) + pos(1) * mvpMat(3, 1) + pos(2) * mvpMat(3, 2) + mvpMat(3, 3);
            double zoomDistance = distance * viewState.get2PowZoom() / viewState.getZoom0Distance();
            drawData->setCameraPlaneZoomDistance(zoomDistance);
    
            if (!_sort3D) {
                // If in 2D, calculate distance from the bottom of the screen
                double dist = GeomUtils::DistanceFromLine(MapPos(pos(0), pos(1), pos(2)), bottomLeft, bottomRight);
                drawData->setScreenBottomDistance(static_cast<int>(dist));
            }
        }
    
        // Sort billboards
        std::sort(_billboardDrawDatas.begin(), _billboardDrawDatas.end(), std::bind(&BillboardSorter::distanceComparator, this, std::placeholders::_1, std::placeholders::_2));
    }
    
    const std::vector<std::shared_ptr<BillboardDrawData> >& BillboardSorter::getSortedBillboardDrawDatas() const {
        return _billboardDrawDatas;
    }
    
    bool BillboardSorter::distanceComparator(const std::shared_ptr<BillboardDrawData>& drawData1, const std::shared_ptr<BillboardDrawData>& drawData2) const {
        // First compare placement priorities
        int priorityDelta = drawData2->getPlacementPriority() - drawData1->getPlacementPriority();
        if (priorityDelta > 0) {
            return true;
        } else if (priorityDelta < 0) {
            return false;
        }
    
        if (_sort3D) {
            // If in 3D, sort the distance to the camera plane
            return drawData1->getCameraPlaneZoomDistance() > drawData2->getCameraPlaneZoomDistance();
        } else {
            // If in 2D, sort by z coordinate and then by the distance to the bottom of the screen
            double zDelta = drawData2->getPos()(2) - drawData1->getPos()(2);
            if (zDelta > 0) {
                return true;
            } else if (zDelta < 0) {
                return false;
            }
            return drawData1->getScreenBottomDistance() > drawData2->getScreenBottomDistance();
        }
    }
    
}
