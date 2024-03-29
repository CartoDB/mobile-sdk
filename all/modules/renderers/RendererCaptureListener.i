#ifndef _RENDERERCAPTURELISTENER_I
#define _RENDERERCAPTURELISTENER_I

%module(directors="1") RendererCaptureListener

!proxy_imports(carto::RendererCaptureListener, graphics.Bitmap)

%{
#include "renderers/RendererCaptureListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"

!polymorphic_shared_ptr(carto::RendererCaptureListener, renderers.RendererCaptureListener)

%feature("director") carto::RendererCaptureListener;

%include "renderers/RendererCaptureListener.h"

#endif
