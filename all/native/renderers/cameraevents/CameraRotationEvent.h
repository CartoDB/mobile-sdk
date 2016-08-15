/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CAMERAROTATIONEVENT_H_
#define _CARTO_CAMERAROTATIONEVENT_H_

#include "core/MapPos.h"
#include "renderers/cameraevents/CameraEvent.h"

namespace carto {

    class CameraRotationEvent : public CameraEvent {
    public:
        CameraRotationEvent();
        virtual ~CameraRotationEvent();
    
        float getRotation() const;
        void setRotation(float rotation);
    
        float getRotationDelta() const;
        void setRotationDelta(float rotationDelta);
        void setRotationDelta(double sin, double cos);
        
        const MapPos& getTargetPos() const;
        void setTargetPos(const MapPos& targetPos);
    
        bool isUseDelta() const;
        bool isUseTarget() const;
    
        void calculate(Options& options, ViewState& viewState);
        
    private:
        float _rotation;
    
        double _sin;
        double _cos;
    
        MapPos _targetPos;
    
        bool _useDelta;
        bool _useTarget;
    };
    
}

#endif
