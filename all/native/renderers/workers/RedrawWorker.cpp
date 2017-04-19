#include "RedrawWorker.h"
#include "renderers/MapRenderer.h"
#include "renderers/RedrawRequestListener.h"
#include "utils/Log.h"
#include "utils/ThreadUtils.h"

namespace carto {

    RedrawWorker::RedrawWorker() :
        _wakeupMap(),
        _mapRenderer(),
        _stop(false),
        _idle(false),
        _condition(),
        _mutex()
    {
    }
    
    RedrawWorker::~RedrawWorker() {
    }
        
    void RedrawWorker::setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<RedrawWorker>& worker) {
        _mapRenderer = mapRenderer;
        // When the map component gets destroyed all threads get detatched. Detatched threads need their worker objects to be alive,
        // so worker objects need to keep references to themselves, until the loop finishes.
        _worker = worker;
    }
    
    void RedrawWorker::init(const DirectorPtr<RedrawRequestListener>& listener) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        std::chrono::steady_clock::time_point wakeupTime = std::chrono::steady_clock::now();
        if (_wakeupMap.find(listener) != _wakeupMap.end()) {
            return;
        }
        _wakeupMap[listener] = wakeupTime;
        _idle = false;
        _condition.notify_one();
    }
    
    void RedrawWorker::stop() {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true;
        _condition.notify_all();
    }
    
    bool RedrawWorker::isIdle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _idle;
    }
    
    void RedrawWorker::operator ()() {
        run();
        _worker.reset();
    }
        
    void RedrawWorker::run() {
        ThreadUtils::SetThreadPriority(ThreadPriority::HIGH);
        while (true) {
            std::vector<DirectorPtr<RedrawRequestListener> > listeners;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                
                if (_stop) {
                    return;
                }

                std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
                std::chrono::steady_clock::time_point wakeupTime = std::chrono::steady_clock::now() + std::chrono::hours(24);
                for (auto it = _wakeupMap.begin(); it != _wakeupMap.end(); ) {
                    if (it->second - currentTime < std::chrono::milliseconds(1)) {
                        listeners.push_back(it->first);
                        it = _wakeupMap.erase(it);
                    } else {
                        wakeupTime = std::min(wakeupTime, it->second);
                        it++;
                    }
                }
                
                if (listeners.empty()) {
                    _idle = _wakeupMap.empty();
                    _condition.wait_for(lock, wakeupTime - std::chrono::steady_clock::now());
                    _idle = false;
                }
            }

            for (const DirectorPtr<RedrawRequestListener>& listener : listeners) {
                listener->onRedrawRequested();
            }
        }
    }
    
}
