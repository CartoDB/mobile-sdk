/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TOUCHHANDLER_H_
#define _CARTO_TOUCHHANDLER_H_

#include "core/MapPos.h"
#include "core/ScreenPos.h"
#include "core/MapVec.h"
#include "ui/MapClickInfo.h"
#include "renderers/MapRenderer.h"

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <cglib/vec.h>

namespace carto {
    class ClickHandlerWorker;
    class Options;
    class MapEventListener;
    class ViewState;
    
    class TouchHandler : public std::enable_shared_from_this<TouchHandler> {
    public:
        enum {
            ACTION_POINTER_1_DOWN = 0,
            ACTION_POINTER_2_DOWN = 1,
            ACTION_MOVE = 2,
            ACTION_CANCEL = 3,
            ACTION_POINTER_1_UP = 4,
            ACTION_POINTER_2_UP = 5
        };

        struct OnTouchListener {
            virtual ~OnTouchListener() { }

            virtual bool onTouchEvent(int action, const ScreenPos& screenPos1, const ScreenPos& screenPos2) = 0;
        };
    
        TouchHandler(const std::shared_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<Options>& options);
        virtual ~TouchHandler();

        void init();
        void deinit();
    
        std::shared_ptr<MapEventListener> getMapEventListener() const;
        void setMapEventListener(const std::shared_ptr<MapEventListener>& mapEventListener);
        
        void onTouchEvent(int action, const ScreenPos& screenPos1, const ScreenPos& screenPos2);
    
        void click(const ScreenPos& screenPos) const;
        void longClick(const ScreenPos& screenPos);
        void doubleClick(const ScreenPos& screenPos) const;
        void dualClick(const ScreenPos& screenPos1, const ScreenPos& screenPos2) const;
        void startSinglePointer(const ScreenPos& screenPos);
        void startDualPointer(const ScreenPos& screenPos1, const ScreenPos& screenPos2);

        void registerOnTouchListener(const std::shared_ptr<OnTouchListener>& listener);
        void unregisterOnTouchListener(const std::shared_ptr<OnTouchListener>& listener);
        
    protected:
        friend class BaseMapView;
    
    private:
        class MapRendererListener : public MapRenderer::OnChangeListener {
        public:
            MapRendererListener(const std::shared_ptr<TouchHandler>& touchHandler);
            
            virtual void onMapChanged();
            virtual void onMapIdle();
            
        private:
            std::weak_ptr<TouchHandler> _touchHandler;
        };
        
        void checkMapStable();

        bool isValidTouchPosition(const MapPos& mapPos, const ViewState& viewState) const;

        float calculateRotatingScalingFactor(const ScreenPos& screenPos1, const ScreenPos& screenPos2) const;

        void singlePointerPan(const ScreenPos& screenPos);
        void dualPointerGuess(const ScreenPos& screenPos1, const ScreenPos& screenPos2);
        void dualPointerTilt(const ScreenPos& screenPos);
        void dualPointerPan(const ScreenPos& screenPos1, const ScreenPos& screenPos2, bool rotate, bool scale);

        void handleClick(ClickType::ClickType clickType, const MapPos& targetPos) const;
    
        static const int SINGLE_POINTER_CLICK_GUESS = 0;
        static const int DUAL_POINTER_CLICK_GUESS = 1;
        static const int SINGLE_POINTER_PAN = 2;
        static const int DUAL_POINTER_GUESS = 3;
        static const int DUAL_POINTER_TILT = 4;
        static const int DUAL_POINTER_ROTATE = 5;
        static const int DUAL_POINTER_SCALE = 6;
        static const int DUAL_POINTER_FREE = 7;
    
        static const float GUESS_MAX_DELTA_Y_INCHES;
        static const float GUESS_MIN_SWIPE_LENGTH_SAME_INCHES;
        static const float GUESS_MIN_SWIPE_LENGTH_OPPOSITE_INCHES;
    
        // Determines whether to choose view angle or some other pointer mode
        static const float GUESS_SWIPE_ABS_COS_THRESHOLD;
    
        // Determines when to switch between scalign and rotating when google style
        // panning is enabled
        static const float ROTATION_FACTOR_THRESHOLD;
        static const float SCALING_FACTOR_THRESHOLD;
        static const float ROTATION_SCALING_FACTOR_THRESHOLD_STICKY;
    
        // Determines how the finger sliding distance will be converted to tilt angle
        static const float INCHES_TO_VIEW_ANGLE;
        
        // Determines how long it takes to cancel kinetic zoom and rotation after one
        // pointer is lifted but the other one is not
        static const std::chrono::milliseconds DUAL_KINETIC_HOLD_DURATION;

        // Determines how long to hold panning after one pointer is lifted
        static const std::chrono::milliseconds DUAL_STOP_HOLD_DURATION;
    
        // Map panning type, 0 = fast, accurate (finger stays exactly in the same
        // place), 1 = slow, inaccurate
        static const float PANNING_FACTOR;
    
        int _panningMode;
        
        ScreenPos _prevScreenPos1;
        ScreenPos _prevScreenPos2;
    
        cglib::vec2<float> _swipe1;
        cglib::vec2<float> _swipe2;
    
        int _pointersDown;
        bool _mapMoving;
        bool _noDualPointerYet;
        std::chrono::steady_clock::time_point _dualPointerReleaseTime;
    
        ThreadSafeDirectorPtr<MapEventListener> _mapEventListener;
        
        std::shared_ptr<ClickHandlerWorker> _clickHandlerWorker;
        std::thread _clickHandlerThread;
    
        std::shared_ptr<Options> _options;
        std::shared_ptr<MapRenderer> _mapRenderer;
        std::shared_ptr<MapRendererListener> _mapRendererListener;
    
        mutable std::mutex _mutex;

        std::vector<std::shared_ptr<OnTouchListener> > _onTouchListeners;
        mutable std::mutex _onTouchListenersMutex;
    };
    
}

#endif
