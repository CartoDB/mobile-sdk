#ifndef _REDRAWREQUESTLISTENER_I
#define _REDRAWREQUESTLISTENER_I

%module(directors="1") RedrawRequestListener

%{
#include "renderers/RedrawRequestListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>

!polymorphic_shared_ptr(carto::RedrawRequestListener, renderers.RedrawRequestListener)

%feature("director") carto::RedrawRequestListener;

%include "renderers/RedrawRequestListener.h"

#endif
