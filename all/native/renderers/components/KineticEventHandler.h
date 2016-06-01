/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_KINETICEVENTHANDLER_H_
#define _CARTO_KINETICEVENTHANDLER_H_

#include "core/MapPos.h"
#include "core/MapVec.h"

#include <deque>
#include <mutex>

namespace carto {
    class MapRenderer;
    class Options;
    class ViewState;
    
    class KineticEventHandler {
    public:
        KineticEventHandler(MapRenderer& mapRenderer, Options& options);
        virtual ~KineticEventHandler();
    
        void calculate(const ViewState& viewState, float deltaSeconds);
    
        bool isPanning() const;
        void setPanDelta(const MapVec& deltaFocusPos, float zoom);
        void startPan();
        void stopPan();
    
        bool isRotating() const;
        void setRotationDelta(float deltaRotation, const MapPos& targetPos);
        void startRotation();
        void stopRotation();
        
        bool isZooming() const;
        void setZoomDelta(float deltaZoom, const MapPos& targetPos);
        void startZoom();
        void stopZoom();

    private:
        void handlePan(const ViewState& viewState, float deltaSeconds);
        void handleRotation(const ViewState& viewState, float deltaSeconds);
        void handleZoom(const ViewState& viewState, float deltaSeconds);
    
        static const float KINETIC_PAN_START_TOLERANCE_ZOOM_0;
        static const float KINETIC_PAN_STOP_TOLERANCE_ZOOM_0;
        static const float KINETIC_PAN_SLOWDOWN;
        static const float KINETIC_PAN_DELTA_MULTIPLIER;
    
        static const float KINETIC_ROTATION_START_TOLERANCE_ANGLE;
        static const float KINETIC_ROTATION_STOP_TOLERANCE_ANGLE;
        static const float KINETIC_ROTATION_SLOWDOWN;
        static const float KINETIC_ROTATION_DELTA_MULTIPLIER;
        
        static const float KINETIC_ZOOM_START_TOLERANCE;
        static const float KINETIC_ZOOM_STOP_TOLERANCE;
        static const float KINETIC_ZOOM_SLOWDOWN;
        static const float KINETIC_ZOOM_DELTA_MULTIPLIER;
        
        static const int AVERAGE_SAMPLE_COUNT = 7;
    
        bool _pan;
        MapVec _panDelta;
    
        bool _rotation;
        float _rotationDelta;
        MapPos _rotationTargetPos;
        std::deque<float> _rotationDeltaSamples;
        
        bool _zoom;
        float _zoomDelta;
        MapPos _zoomTargetPos;
        std::deque<float> _zoomDeltaSamples;
    
        MapRenderer& _mapRenderer;
        Options& _options;
    
        mutable std::mutex _mutex;
    };
    
}

#endif
