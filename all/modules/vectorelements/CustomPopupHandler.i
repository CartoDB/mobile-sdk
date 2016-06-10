#ifndef _CUSTOMPOPUPHANDLER_I
#define _CUSTOMPOPUPHANDLER_I

%module(directors="1") CustomPopupHandler

!proxy_imports(carto::CustomPopupHandler, graphics.Bitmap, ui.PopupDrawInfo, ui.PopupClickInfo)

%{
#include "vectorelements/CustomPopupHandler.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"
%import "ui/PopupDrawInfo.i"
%import "ui/PopupClickInfo.i"

!polymorphic_shared_ptr(carto::CustomPopupHandler, vectorelements.CustomPopupHandler)

%feature("director") carto::CustomPopupHandler;

%include "vectorelements/CustomPopupHandler.h"

#endif
