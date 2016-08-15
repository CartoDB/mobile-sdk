/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CAMERATILTEVENT_H_
#define _CARTO_CAMERATILTEVENT_H_

#include "renderers/cameraevents/CameraEvent.h"

namespace carto {

    class CameraTiltEvent : public CameraEvent {
    public:
        CameraTiltEvent();
        virtual ~CameraTiltEvent();
    
        float getTilt() const;
        void setTilt(float tilt);
    
        float getTiltDelta() const;
        void setTiltDelta(float tiltDelta);
    
        bool isUseDelta() const;
    
        void calculate(Options& options, ViewState& viewState);
    private:
        float _tilt;
    
        float _tiltDelta;
    
        bool _useDelta;
    };
    
}

#endif
