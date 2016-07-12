#ifndef _TILELOADLISTENER_I
#define _TILELOADLISTENER_I

%module(directors="1") TileLoadListener

%{
#include "layers/TileLoadListener.h"	
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>

!polymorphic_shared_ptr(carto::TileLoadListener, layers.TileLoadListener)

%feature("director") carto::TileLoadListener;

%include "layers/TileLoadListener.h"

#endif
