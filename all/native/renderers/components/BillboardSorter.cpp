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
    
        // Calculate the world positions of the bottom screen corners, offset them by a little to avoid certain artifacts
        cglib::vec3<double> bottomLeft = viewState.screenToWorldPlane(cglib::vec2<float>(0, viewState.getHeight() * 1.5f), 0);
        cglib::vec3<double> bottomRight = viewState.screenToWorldPlane(cglib::vec2<float>(viewState.getWidth(), viewState.getHeight() * 1.5f), 0);
        cglib::vec3<double> bottomAxis = cglib::unit(bottomRight - bottomLeft);
        bool sort3D = viewState.getTilt() < 90;
    
        // Calculate billboard distances
        const cglib::mat4x4<double>& mvpMat = viewState.getModelviewProjectionMat();
        for (const std::shared_ptr<BillboardDrawData>& drawData : _billboardDrawDatas) {
            const cglib::vec3<double>& pos = drawData->getPos();
    
            // Calculate distance to the camera plane, adjust to zoom
            double distance = pos(0) * mvpMat(3, 0) + pos(1) * mvpMat(3, 1) + pos(2) * mvpMat(3, 2) + mvpMat(3, 3);
            double zoomDistance = distance * viewState.get2PowZoom() / viewState.getZoom0Distance();
            drawData->setCameraPlaneZoomDistance(zoomDistance);
    
            if (!sort3D) {
                // If in 2D, calculate distance from the bottom of the screen
                cglib::vec3<double> projPos = bottomLeft + bottomAxis * cglib::dot_product(pos - bottomLeft, bottomAxis);
                drawData->setScreenBottomDistance(cglib::length(projPos - pos));
            }
        }

        // Billboard sorter comparator
        auto distanceComparator = [sort3D](const std::shared_ptr<BillboardDrawData>& drawData1, const std::shared_ptr<BillboardDrawData>& drawData2) {
            // First compare placement priorities
            float priorityDelta = drawData2->getPlacementPriority() - drawData1->getPlacementPriority();
            if (priorityDelta > 0) {
                return true;
            } else if (priorityDelta < 0) {
                return false;
            }
    
            if (sort3D) {
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
        };
    
        // Sort billboards
        std::stable_sort(_billboardDrawDatas.begin(), _billboardDrawDatas.end(), distanceComparator);
    }
    
    const std::vector<std::shared_ptr<BillboardDrawData> >& BillboardSorter::getSortedBillboardDrawDatas() const {
        return _billboardDrawDatas;
    }
    
}
