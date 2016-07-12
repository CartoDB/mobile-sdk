#ifndef _VECTORELEMENTEVENTLISTENER_I
#define _VECTORELEMENTEVENTLISTENER_I

%module(directors="1") VectorElementEventListener

!proxy_imports(carto::VectorElementEventListener, ui.VectorElementClickInfo)

%{
#include "layers/VectorElementEventListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/VectorElementClickInfo.i"

!polymorphic_shared_ptr(carto::VectorElementEventListener, layers.VectorElementEventListener)

%feature("director") carto::VectorElementEventListener;

%include "layers/VectorElementEventListener.h"

#endif
