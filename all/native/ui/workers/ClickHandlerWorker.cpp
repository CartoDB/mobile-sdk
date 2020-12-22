#include "ClickHandlerWorker.h"
#include "components/Options.h"
#include "ui/TouchHandler.h"
#include "utils/Log.h"
#include "utils/ThreadUtils.h"

#include <cmath>

namespace carto {

    ClickHandlerWorker::ClickHandlerWorker(const std::shared_ptr<Options>& options) :
        _running(false),
        _clickMode(NO_CLICK),
        _clickTypeDetection(false),
        _startTime(),
        _pointersDown(0),
        _pointer1Down(0, 0),
        _pointer1Moved(0, 0),
        _pointer1MovedSum(0),
        _pointer2Down(0, 0),
        _pointer2Moved(0, 0),
        _pointer2MovedSum(0),
        _chosen(false),
        _canceled(false),
        _options(options),
        _touchHandler(),
        _stop(false),
        _condition(),
        _mutex()
    {
    }
    
    ClickHandlerWorker::~ClickHandlerWorker() {
    }
        
    void ClickHandlerWorker::setComponents(const std::weak_ptr<TouchHandler>& touchHandler, const std::shared_ptr<ClickHandlerWorker>& worker) {
        _touchHandler = touchHandler;
        // When the map component gets destroyed all threads get detatched. Detatched threads need their worker objects to be alive,
        // so worker objects need to keep references to themselves, until the loop finishes.
        _worker = worker;
    }
    
    bool ClickHandlerWorker::isRunning() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _running;
    }
    
    void ClickHandlerWorker::init() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_running) {
            return;
        }
    
        _clickTypeDetection = _options->isClickTypeDetection();
        _startTime = std::chrono::steady_clock::now();
    
        _clickMode = LONG_CLICK;
    
        _pointersDown = 0;
    
        _pointer1MovedSum = 0;
        _pointer2MovedSum = 0;
    
        _chosen = false;
        _canceled = false;
    
        _running = true;
    
        _condition.notify_one();
    }
    
    void ClickHandlerWorker::stop() {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true;
        _condition.notify_all();
    }
    
    void ClickHandlerWorker::pointer1Down(const ScreenPos& screenPos) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_chosen) {
            return;
        }
    
        _pointersDown++;
        
        if (_clickMode == DOUBLE_CLICK) {
            cglib::vec2<float> delta(screenPos.getX() - _pointer1Down.getX(), screenPos.getY() - _pointer1Down.getY());
            if (cglib::length(delta) / _options->getDPI() < DOUBLE_CLICK_TOLERANCE_INCHES) {
                _chosen = true;
            }
        }
        
        _pointer1Down = screenPos;
        _pointer1Moved = _pointer1Down;
        _pointer1MovedSum = 0;
    }
    
    void ClickHandlerWorker::pointer1Moved(const ScreenPos& screenPos) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_chosen) {
            return;
        }
    
        _pointer1MovedSum += std::abs(screenPos.getX() - _pointer1Moved.getX());
        _pointer1MovedSum += std::abs(screenPos.getY() - _pointer1Moved.getY());
        _pointer1Moved = screenPos;
    
        float dpi = _options->getDPI();
        if (_clickMode == LONG_CLICK) {
            if (_pointer1MovedSum / dpi >= MOVING_TOLERANCE_INCHES) {
                _chosen = true;
                _canceled = true;
            }
        } else if (_clickMode == DUAL_CLICK) {
            if (_pointer1MovedSum / dpi >= MOVING_TOLERANCE_INCHES && _pointersDown == 2) {
                _chosen = true;
                _canceled = true;
            }
        }
    }
    
    void ClickHandlerWorker::pointer1Up() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_chosen) {
            return;
        }
    
        _pointersDown--;
    
        if (_clickMode == LONG_CLICK) {
            _clickMode = DOUBLE_CLICK;
        } else if (_clickMode == DUAL_CLICK) {
            if (_pointersDown == 0) {
                _chosen = true;
            }
        }
    }
    
    void ClickHandlerWorker::pointer2Down(const ScreenPos& screenPos) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_chosen) {
            return;
        }
    
        _pointersDown++;
    
        _pointer2Down = screenPos;
        _pointer2Moved = _pointer2Down;
        _pointer2MovedSum = 0;
    
        _clickMode = DUAL_CLICK;
        auto deltaTime = std::chrono::steady_clock::now() - _startTime;
        if (deltaTime > DUAL_CLICK_BEGIN_DURATION) {
            _chosen = true;
            _canceled = true;
        }
    }
    
    void ClickHandlerWorker::pointer2Moved(const ScreenPos& screenPos) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_chosen) {
            return;
        }
    
        _pointer2MovedSum += std::abs(screenPos.getX() - _pointer2Moved.getX());
        _pointer2MovedSum += std::abs(screenPos.getY() - _pointer2Moved.getY());
    
        _pointer2Moved = screenPos;
    
        float dpi = _options->getDPI();
        if (_clickMode == DUAL_CLICK) {
            if (_pointer2MovedSum / dpi >= MOVING_TOLERANCE_INCHES && _pointersDown == 2) {
                _chosen = true;
                _canceled = true;
            }
        }
    }
    
    void ClickHandlerWorker::pointer2Up() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_chosen) {
            return;
        }
    
        _pointersDown--;
    }
    
    void ClickHandlerWorker::cancel() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_chosen) {
            return;
        }
    
        _clickMode = NO_CLICK;
        _chosen = true;
    }
    
    void ClickHandlerWorker::operator()() {
        run();
        _worker.reset();
    }

    void ClickHandlerWorker::run() {
        ThreadUtils::SetThreadPriority(ThreadPriority::LOW);

        while (true) {
            // If not running, wait until notified or exit thread if interrupted
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_stop) {
                    return;
                }
                if (!_running) {
                    _condition.wait(lock);
                }
            }
            
            ClickMode clickMode = NO_CLICK;
            while (true) {
                // Loop until a click type is chosen
                {
                    std::lock_guard<std::mutex> lock(_mutex);

                    if (_stop) {
                        return;
                    }
                    if (_chosen) {
                        clickMode = _clickMode;
                        break;
                    }
                    
                    auto deltaTime = std::chrono::steady_clock::now() - _startTime;
                    switch (_clickMode) {
                    case NO_CLICK:
                        _chosen = true;
                        break;
                    case LONG_CLICK:
                        if (!_clickTypeDetection || deltaTime >= LONG_CLICK_MIN_DURATION) {
                            _chosen = true;
                        }
                        break;
                    case DOUBLE_CLICK:
                        if (!_clickTypeDetection || deltaTime >= DOUBLE_CLICK_MAX_DURATION) {
                            _chosen = true;
                            _canceled = true;
                        }
                        break;
                    case DUAL_CLICK:
                        if (!_clickTypeDetection || deltaTime >= DUAL_CLICK_END_DURATION) {
                            _chosen = true;
                            _canceled = true;
                        }
                        break;
                    }
                }
                std::this_thread::yield();
            }
            
            switch (clickMode) {
            case NO_CLICK:
                break;
            case LONG_CLICK:
                afterLongClick();
                break;
            case DOUBLE_CLICK:
                afterDoubleClick();
                break;
            case DUAL_CLICK:
                afterDualClick();
                break;
            }
            
            {
                // Click type was detected, stop running
                std::lock_guard<std::mutex> lock(_mutex);
                _running = false;
                _clickMode = NO_CLICK;
            }
        }
    }

    void ClickHandlerWorker::afterLongClick() {
        std::shared_ptr<TouchHandler> touchHandler = _touchHandler.lock();
        if (!touchHandler) {
            return;
        }
        
        std::unique_lock<std::mutex> lock(_mutex);
        bool canceled = _canceled;
        bool clickTypeDetection = _clickTypeDetection;
        ScreenPos pointer1Down = _pointer1Down;
        ScreenPos pointer1Moved = _pointer1Moved;
        lock.unlock();

        if (!clickTypeDetection) {
            touchHandler->startSinglePointer(pointer1Moved);
            touchHandler->click(pointer1Down);
            return;
        }
    
        if (canceled) {
            touchHandler->startSinglePointer(pointer1Moved);
            return;
        }

        touchHandler->longClick(pointer1Down);
    }
    
    void ClickHandlerWorker::afterDoubleClick() {
        std::shared_ptr<TouchHandler> touchHandler = _touchHandler.lock();
        if (!touchHandler) {
            return;
        }

        std::unique_lock<std::mutex> lock(_mutex);
        bool canceled = _canceled;
        ScreenPos pointer1Down = _pointer1Down;
        lock.unlock();
        
        if (canceled) {
            touchHandler->click(pointer1Down);
            return;
        }

        touchHandler->doubleClick(pointer1Down);
    }
    
    void ClickHandlerWorker::afterDualClick() {
        std::shared_ptr<TouchHandler> touchHandler = _touchHandler.lock();
        if (!touchHandler) {
            return;
        }

        std::unique_lock<std::mutex> lock(_mutex);
        bool canceled = _canceled;
        int pointersDown = _pointersDown;
        ScreenPos pointer1Down = _pointer1Down;
        ScreenPos pointer2Down = _pointer2Down;
        ScreenPos pointer1Moved = _pointer1Moved;
        ScreenPos pointer2Moved = _pointer2Moved;
        lock.unlock();
        
        if (canceled) {
            if (pointersDown == 1) {
                touchHandler->startSinglePointer(pointer1Moved);
            } else if (pointersDown >= 2) {
                touchHandler->startDualPointer(pointer1Moved, pointer2Moved);
            }
            return;
        }

        touchHandler->dualClick(pointer1Down, pointer2Down);
    }
        
    const std::chrono::milliseconds ClickHandlerWorker::LONG_CLICK_MIN_DURATION = std::chrono::milliseconds(400);
    const std::chrono::milliseconds ClickHandlerWorker::DUAL_CLICK_BEGIN_DURATION = std::chrono::milliseconds(100);
    const std::chrono::milliseconds ClickHandlerWorker::DUAL_CLICK_END_DURATION = std::chrono::milliseconds(300);
    const std::chrono::milliseconds ClickHandlerWorker::DOUBLE_CLICK_MAX_DURATION = std::chrono::milliseconds(400);
        
    const float ClickHandlerWorker::DOUBLE_CLICK_TOLERANCE_INCHES = 1.3f;
    const float ClickHandlerWorker::MOVING_TOLERANCE_INCHES = 0.2f;
    
}

