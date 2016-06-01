#ifndef _MAPRENDERERLISTENER_I
#define _MAPRENDERERLISTENER_I

%module(directors="1") MapRendererListener

%{
#include "renderers/MapRendererListener.h"
#include "graphics/Bitmap.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>

!polymorphic_shared_ptr(carto::MapRendererListener, renderers.MapRendererListener)

%feature("director") carto::MapRendererListener;

%include "renderers/MapRendererListener.h"

#endif
