/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_REDRAWWORKER_H_
#define _CARTO_REDRAWWORKER_H_

#include "components/DirectorPtr.h"
#include "components/ThreadWorker.h"

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <thread>
#include <vector>

namespace carto {
    class MapRenderer;
    class Options;
    class RedrawRequestListener;
    
    class RedrawWorker : public ThreadWorker {
    public:
        RedrawWorker();
        virtual ~RedrawWorker();
        
        void setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<RedrawWorker>& worker);
    
        void init(const DirectorPtr<RedrawRequestListener>& listener);
        
        void stop();
        
        bool isIdle() const;
    
        void operator()();
    
    private:
        void run();
    
        std::map<DirectorPtr<RedrawRequestListener>, std::chrono::steady_clock::time_point> _wakeupMap;
        
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::shared_ptr<RedrawWorker> _worker;
    
        bool _stop;
        bool _idle;
        std::condition_variable _condition;
        mutable std::mutex _mutex;
    };
    
}

#endif
