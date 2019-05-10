#ifndef _BALLOONPOPUPEVENTLISTENER_I
#define _BALLOONPOPUPEVENTLISTENER_I

%module(directors="1") BalloonPopupEventListener

!proxy_imports(carto::BalloonPopupEventListener, ui.BalloonPopupButtonClickInfo)

%{
#include "vectorelements/BalloonPopupEventListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/BalloonPopupButtonClickInfo.i"

!polymorphic_shared_ptr(carto::BalloonPopupEventListener, vectorelements.BalloonPopupEventListener)

%feature("director") carto::BalloonPopupEventListener;

%include "vectorelements/BalloonPopupEventListener.h"

#endif
