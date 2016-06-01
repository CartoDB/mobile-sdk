/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BILLBOARDPLACEMENTWORKER_H_
#define _CARTO_BILLBOARDPLACEMENTWORKER_H_

#include "components/ThreadWorker.h"
#include "core/MapEnvelope.h"
#include "geometry/utils/KDTreeSpatialIndex.h"

#include <condition_variable>
#include <memory>
#include <mutex>

namespace carto {
    class Billboard;
    class BillboardDrawData;
    class MapRenderer;
    
    class BillboardPlacementWorker : public ThreadWorker {
    public:
        BillboardPlacementWorker();
        virtual ~BillboardPlacementWorker();
        
        void setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<BillboardPlacementWorker>& worker);
        
        void init(int delayTime);
        
        void stop();
        
        bool isIdle() const;
    
        void operator()();
    
    private:
        void run();
        
        bool calculateBillboardPlacement();
        
        bool overlapComparator(const std::shared_ptr<BillboardDrawData>& drawData1, const std::shared_ptr<BillboardDrawData>& drawData2) const;
        
        bool _stop;
        bool _idle;
        
        KDTreeSpatialIndex<MapEnvelope> _kdTree;
        
        bool _sort3D;
    
        bool _pendingWakeup;
        std::chrono::steady_clock::time_point _wakeupTime;
        
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::shared_ptr<BillboardPlacementWorker> _worker;
    
        std::condition_variable _condition;
        mutable std::mutex _mutex;
    };
    
}

#endif
