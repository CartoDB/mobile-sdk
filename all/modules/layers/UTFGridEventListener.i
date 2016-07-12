#ifndef _UTFGRIDEVENTLISTENER_I
#define _UTFGRIDEVENTLISTENER_I

%module(directors="1") UTFGridEventListener

!proxy_imports(carto::UTFGridEventListener, ui.UTFGridClickInfo)

%{
#include "layers/UTFGridEventListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/UTFGridClickInfo.i"

!polymorphic_shared_ptr(carto::UTFGridEventListener, layers.UTFGridEventListener)

%feature("director") carto::UTFGridEventListener;

%include "layers/UTFGridEventListener.h"

#endif
