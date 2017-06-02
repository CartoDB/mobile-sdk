/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CULLWORKER_H_
#define _CARTO_CULLWORKER_H_

#include "components/ThreadWorker.h"
#include "core/MapEnvelope.h"
#include "graphics/Frustum.h"
#include "renderers/components/CullState.h"

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <thread>
#include <vector>

namespace carto {
    class EnvelopeLayer;
    class Layer;
    class MapRenderer;
    class Options;
    
    class CullWorker : public ThreadWorker {
    public:
        CullWorker();
        virtual ~CullWorker();
        
        void setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<CullWorker>& worker);
    
        void init(const std::shared_ptr<Layer>& layer, int delayTime);
        
        void stop();
        
        bool isIdle() const;
    
        void operator()();
    
    private:
        void run();
    
        void calculateCullState();
        void calculateEnvelope();
        void updateLayers(const std::vector<std::shared_ptr<Layer> >& layers);
    
        static const float VIEWPORT_SCALE;

        std::map<std::shared_ptr<Layer>, std::chrono::steady_clock::time_point> _layerWakeupMap;
        
        bool _firstCull;
        
        MapEnvelope _envelope;
        
        ViewState _viewState;
    
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::shared_ptr<CullWorker> _worker;
    
        bool _stop;
        bool _idle;
        std::condition_variable _condition;
        mutable std::mutex _mutex;
    };
    
}

#endif
