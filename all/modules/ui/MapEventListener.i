#ifndef _MAPEVENTLISTENER_I
#define _MAPEVENTLISTENER_I

%module(directors="1") MapEventListener

!proxy_imports(carto::MapEventListener, ui.MapClickInfo, ui.MapInteractionInfo)

%{
#include "ui/MapEventListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/MapClickInfo.i"
%import "ui/MapInteractionInfo.i"

!polymorphic_shared_ptr(carto::MapEventListener, ui.MapEventListener)

%feature("director") carto::MapEventListener;

%include "ui/MapEventListener.h"

#endif
