#include "BillboardPlacementWorker.h"
#include "renderers/BillboardRenderer.h"
#include "renderers/MapRenderer.h"
#include "renderers/drawdatas/BillboardDrawData.h"
#include "utils/Log.h"
#include "utils/ThreadUtils.h"
#include "vectorelements/Billboard.h"

#include <algorithm>

namespace carto {

    BillboardPlacementWorker::BillboardPlacementWorker() :
        _stop(false),
        _idle(false),
        _kdTree(),
        _sort3D(false),
        _pendingWakeup(false),
        _wakeupTime(std::chrono::steady_clock::now() + std::chrono::hours(24)),
        _mapRenderer(),
        _condition(),
        _mutex()
    {
    }
    
    BillboardPlacementWorker::~BillboardPlacementWorker() {
    }
        
    void BillboardPlacementWorker::setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<BillboardPlacementWorker>& worker)
    {
        _mapRenderer = mapRenderer;
        // When the map component gets destroyed all threads get detatched. Detatched threads need their worker objects to be alive,
        // so worker objects need to keep references to themselves, until the loop finishes.
        _worker = worker;
    }
        
    void BillboardPlacementWorker::init(int delayTime) {
        std::lock_guard<std::mutex> lock(_mutex);
        _idle = false;
        _pendingWakeup = true;
        _wakeupTime = std::min(_wakeupTime, std::chrono::steady_clock::now() + std::chrono::milliseconds(delayTime));
        _condition.notify_one();
    }
    
    void BillboardPlacementWorker::stop() {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true;
        _condition.notify_all();
    }
    
    bool BillboardPlacementWorker::isIdle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _idle;
    }
        
    void BillboardPlacementWorker::operator ()() {
        run();
        _worker.reset();
    }
    
    void BillboardPlacementWorker::run() {
        ThreadUtils::SetThreadPriority(ThreadPriority::LOW);
    
        while (true) {
            bool run = false;
            {
                std::unique_lock<std::mutex> lock(_mutex);

                if (_stop) {
                    return;
                }

                std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
                if (_wakeupTime - currentTime < std::chrono::milliseconds(1)) {
                    run = true;
                    _pendingWakeup = false;
                    _wakeupTime = currentTime + std::chrono::hours(24);
                }

                if (!run) {
                    _idle = !_pendingWakeup;
                    _condition.wait_for(lock, _wakeupTime - currentTime);
                    _idle = false;
                }
            }

            if (run) {
                calculateBillboardPlacement();
            }
        }
    }
    
    bool BillboardPlacementWorker::calculateBillboardPlacement() {
        const std::shared_ptr<MapRenderer>& mapRenderer = _mapRenderer.lock();
    
        if (!mapRenderer) {
            return false;
        }
        
        std::vector<std::shared_ptr<BillboardDrawData> > billboardDrawDatas(mapRenderer->getBillboardDrawDatas());
        
        bool calculate = false;
        for (const std::shared_ptr<BillboardDrawData>& drawData : billboardDrawDatas) {
            if (drawData->isHideIfOverlapped()) {
                calculate = true;
                break;
            }
        }
        
        if (!calculate) {
            return false;
        }
        
        ViewState viewState(mapRenderer->getViewState());
        const cglib::mat4x4<float>& rteMVPMat = viewState.getRTEModelviewProjectionMat();
        
        // Sort draw datas
        _sort3D = viewState.getTilt() < 90;
        std::sort(billboardDrawDatas.begin(), billboardDrawDatas.end(), std::bind(&BillboardPlacementWorker::overlapComparator, this, std::placeholders::_1, std::placeholders::_2));
        
        // Calculate billboard screen coordinates, add envelopes to the tree
        std::vector<float> coordBuf(12);
        std::vector<MapPos> convexHull;
        convexHull.reserve(4);
        bool changed = false;
        for (const std::shared_ptr<BillboardDrawData>& drawData : billboardDrawDatas) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_stop) {
                return false;
            }
            
            // Calculate billboard world coordinates
            BillboardRenderer::CalculateBillboardCoords(*drawData, viewState, coordBuf, 0);
            
            // Transform the world coordinates to screen coordinates
            cglib::vec3<float> topLeft(cglib::transform_point(cglib::vec3<float>(coordBuf[0], coordBuf[1], coordBuf[2]), rteMVPMat));
            cglib::vec3<float> bottomLeft(cglib::transform_point(cglib::vec3<float>(coordBuf[3], coordBuf[4], coordBuf[5]), rteMVPMat));
            cglib::vec3<float> topRight(cglib::transform_point(cglib::vec3<float>(coordBuf[6], coordBuf[7], coordBuf[8]), rteMVPMat));
            cglib::vec3<float> bottomRight(cglib::transform_point(cglib::vec3<float>(coordBuf[9], coordBuf[10], coordBuf[11]), rteMVPMat));
            
            // Construct convex polygons from the screen coordinatees
            convexHull.emplace_back(topLeft(0), topLeft(1), 0);
            convexHull.emplace_back(bottomLeft(0), bottomLeft(1), 0);
            convexHull.emplace_back(topRight(0), topRight(1), 0);
            convexHull.emplace_back(bottomRight(0), bottomRight(1), 0);
            MapEnvelope envelope(convexHull);
            
            bool overlapped = false;
            if (drawData->isHideIfOverlapped()) {
                // Check that there are higher priority billboards overlapping with this one
                const std::vector<MapEnvelope>& overlappedEnvelopes = _kdTree.query(envelope.getBounds());
                for (const MapEnvelope& overlappedEnvelope : overlappedEnvelopes) {
                    if (overlappedEnvelope.intersects(envelope)) {
                        // Overlapping detected, hide this billboard
                        drawData->setOverlapping(true);
                        changed = true;
                        overlapped = true;
                        break;
                    }
                }
            }
            
            if (!overlapped) {
                drawData->setOverlapping(false);
                changed = true;
                if (drawData->isCausesOverlap()) {
                    _kdTree.insert(envelope.getBounds(), envelope);
                }
            }
            convexHull.clear();
        }
        
        _kdTree.clear();
        
        if (changed) {
            mapRenderer->requestRedraw();
        }
        
        return true;
    }
        
    bool BillboardPlacementWorker::overlapComparator(const std::shared_ptr<BillboardDrawData>& drawData1, const std::shared_ptr<BillboardDrawData>& drawData2) const {
        // Sort by overlappability
        int overlapDelta = (drawData2->isHideIfOverlapped() ? 0 : 1) - (drawData1->isHideIfOverlapped() ? 0 : 1);
        if (overlapDelta > 0) {
            return false;
        } else if (overlapDelta < 0) {
            return true;
        }
    
        // Sort by priority
        int priorityDelta = drawData2->getPlacementPriority() - drawData1->getPlacementPriority();
        if (priorityDelta > 0) {
            return false;
        } else if (priorityDelta < 0) {
            return true;
        }
        
        if (_sort3D) {
            // If in 3D, sort the distance to the camera plane
            return drawData1->getCameraPlaneZoomDistance() < drawData2->getCameraPlaneZoomDistance();
        } else {
            // If in 2D, sort by z coordinate and then by the distance to the bottom of the screen
            double zDelta = drawData2->getPos()(2) - drawData1->getPos()(2);
            if (zDelta > 0) {
                return false;
            } else if (zDelta < 0) {
                return true;
            }
            return drawData1->getScreenBottomDistance() > drawData2->getScreenBottomDistance();
        }
    }
    
}
