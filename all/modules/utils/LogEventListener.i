#ifndef _LOGEVENTLISTENER_I
#define _LOGEVENTLISTENER_I

%module(directors="1") LogEventListener

%{
#include "utils/LogEventListener.h"	
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>

!polymorphic_shared_ptr(carto::LogEventListener, utils.LogEventListener)

%feature("director") carto::LogEventListener;

%include "utils/LogEventListener.h"

#endif
