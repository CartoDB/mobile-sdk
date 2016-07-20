#include "CullWorker.h"
#include "layers/Layer.h"
#include "renderers/MapRenderer.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"
#include "utils/ThreadUtils.h"

namespace carto {

    CullWorker::CullWorker() :
        _layerWakeupMap(),
        _firstCull(true),
        _envelope(),
        _viewState(),
        _mapRenderer(),
        _stop(false),
        _idle(false),
        _condition(),
        _mutex()
    {
    }
    
    CullWorker::~CullWorker() {
    }
        
    void CullWorker::setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<CullWorker>& worker) {
        _mapRenderer = mapRenderer;
        // When the map component gets destroyed all threads get detatched. Detatched threads need their worker objects to be alive,
        // so worker objects need to keep references to themselves, until the loop finishes.
        _worker = worker;
    }
    
    void CullWorker::init(const std::shared_ptr<Layer>& layer, int delayTime) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        std::chrono::steady_clock::time_point wakeupTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayTime);
        if (_layerWakeupMap.find(layer) != _layerWakeupMap.end()) {
            if (_layerWakeupMap[layer] <= wakeupTime) {
                return;
            }
        }
        _layerWakeupMap[layer] = wakeupTime;
        _idle = false;
        _condition.notify_one();
    }
    
    void CullWorker::stop() {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true;
        _condition.notify_all();
    }
    
    bool CullWorker::isIdle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _idle;
    }
    
    void CullWorker::operator ()() {
        run();
        _worker.reset();
    }
        
    void CullWorker::run() {
        ThreadUtils::SetThreadPriority(ThreadPriority::LOW);
        while (true) {
            std::vector<std::shared_ptr<Layer> > layers;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                
                if (_stop) {
                    return;
                }

                std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
                std::chrono::steady_clock::time_point wakeupTime = std::chrono::steady_clock::now() + std::chrono::hours(24);
                for (auto it = _layerWakeupMap.begin(); it != _layerWakeupMap.end(); ) {
                    if (it->second - currentTime < std::chrono::milliseconds(1)) {
                        layers.push_back(it->first);
                        it = _layerWakeupMap.erase(it);
                    } else {
                        wakeupTime = std::min(wakeupTime, it->second);
                        it++;
                    }
                }
                
                if (layers.empty()) {
                    _idle = _layerWakeupMap.empty();
                    _condition.wait_for(lock, wakeupTime - std::chrono::steady_clock::now());
                    _idle = false;
                }
            }

            if (!layers.empty()) {
                const std::shared_ptr<MapRenderer>& mapRenderer = _mapRenderer.lock();
                if (!mapRenderer) {
                    return;
                }
                
                // Get view state
                const ViewState& viewState = mapRenderer->getViewState();
                
                // Check if view state has changed
                if (_firstCull || viewState.getModelviewProjectionMat() != _viewState.getModelviewProjectionMat()) {
                    _firstCull = false;
                    _viewState = viewState;
                    
                    // Calculate tiles
                    calculateCullState();
                }
                
                // Update layers
                updateLayers(layers);
            }
        }
    }
    
    void CullWorker::calculateCullState() {
        // Calculate envelope for the visible frustum
        calculateEnvelope();
    }
    
    void CullWorker::calculateEnvelope() {
        cglib::mat4x4<double> invMVPMat = cglib::inverse(_viewState.getModelviewProjectionMat());
        
        // Iterate over all edges and find intersection with ground plane
        std::vector<MapPos> hullPoints;
        hullPoints.reserve(12);
        for (int i = 0; i < 12; i++) {
            int ax = (i >> 2) & 3;
            int as = (4-ax) >> 2;
            int bs = (5-ax) >> 2;
            int va = ((i&1) << as) | ((i&2) << bs);
            int vb = va | (1 << ax);
            
            // Calculate vertex 0
            double xa = (va & 1) == 0 ? VIEWPORT_SCALE : -VIEWPORT_SCALE;
            double ya = (va & 2) == 0 ? VIEWPORT_SCALE : -VIEWPORT_SCALE;
            double za = (va & 4) == 0 ? 1 : -1;
            cglib::vec3<double> p0 = cglib::transform_point(cglib::vec3<double>(xa, ya, za), invMVPMat);
            
            // Calculate vertex 1
            double xb = (vb & 1) == 0 ? VIEWPORT_SCALE : -VIEWPORT_SCALE;
            double yb = (vb & 2) == 0 ? VIEWPORT_SCALE : -VIEWPORT_SCALE;
            double zb = (vb & 4) == 0 ? 1 : -1;
            cglib::vec3<double> p1 = cglib::transform_point(cglib::vec3<double>(xb, yb, zb), invMVPMat);
            
            // Calculate intersection between z=0 plane and line p0<->p1
            cglib::vec3<double> dp = p1 - p0;
            if (dp(2) != 0) {
                double t = -p0(2) / dp(2);
                
                // If the intersection point is inside of the frustum, add it to the list of candidate points
                if (t >= 0 && t <= 1) {
                    cglib::vec3<double> hullPoint = p0 + dp * t;
                    hullPoints.emplace_back(hullPoint(0), hullPoint(1), 0);
                }
            }
        }

        // Calculate convex hull of the resulting point set
        std::vector<MapPos> convexHull = GeomUtils::CalculateConvexHull(hullPoints);
        _envelope = MapEnvelope(convexHull);
    }
    
    void CullWorker::updateLayers(const std::vector<std::shared_ptr<Layer> >& layers) {
        for (const std::shared_ptr<Layer>& layer : layers) {
            layer->update(std::make_shared<CullState>(_envelope, _viewState));
        }
    }
    
    const float CullWorker::VIEWPORT_SCALE = 1.1f; // enlarge viewport envelope by approx. 10%
}
