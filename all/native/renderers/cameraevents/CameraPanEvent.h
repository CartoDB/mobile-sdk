/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CAMERAPANEVENT_H_
#define _CARTO_CAMERAPANEVENT_H_

#include "core/MapPos.h"
#include "core/MapVec.h"
#include "renderers/cameraevents/CameraEvent.h"

namespace carto {

    class CameraPanEvent : public CameraEvent {
    public:
        CameraPanEvent();
        virtual ~CameraPanEvent();
    
        const MapPos& getPos() const;
        void setPos(const MapPos& pos);
    
        const MapVec& getPosDelta() const;
        void setPosDelta(const MapVec& posDelta);
    
        bool isUseDelta() const;
    
        void calculate(Options& options, ViewState& viewState);
    
    private:
        MapPos _pos;
    
        MapVec _posDelta;
    
        bool _useDelta;
    };
    
}

#endif
