/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VTLABELPLACEMENTWORKER_H_
#define _CARTO_VTLABELPLACEMENTWORKER_H_

#include "components/ThreadWorker.h"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace carto {
    class Layer;
    class MapRenderer;
    
    class VTLabelPlacementWorker : public ThreadWorker {
    public:
        VTLabelPlacementWorker();
        virtual ~VTLabelPlacementWorker();
        
        void setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<VTLabelPlacementWorker>& worker);
        
        void init(const std::shared_ptr<Layer>& layer, int delayTime);
        
        void stop();
        
        bool isIdle() const;
    
        void operator()();
    
    private:
        void run();
        
        bool calculateVTLabelPlacement();
        
        bool _stop;
        bool _idle;
        
        bool _pendingWakeup;
        std::chrono::steady_clock::time_point _wakeupTime;
        
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::shared_ptr<VTLabelPlacementWorker> _worker;
    
        std::condition_variable _condition;
        mutable std::mutex _mutex;
    };
    
}

#endif
