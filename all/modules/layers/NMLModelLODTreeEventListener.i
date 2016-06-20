#ifndef _NMLMODELLODTREEEVENTLISTENER_I
#define _NMLMODELLODTREEEVENTLISTENER_I

%module(directors="1") NMLModelLODTreeEventListener

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

!proxy_imports(carto::NMLModelLODTreeEventListener, ui.NMLModelLODTreeClickInfo)


%{
#include "layers/NMLModelLODTreeEventListener.h"
#include "ui/NMLModelLODTreeClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>

%import "ui/NMLModelLODTreeClickInfo.i"

!polymorphic_shared_ptr(carto::NMLModelLODTreeEventListener, layers.NMLModelLODTreeEventListener)

%feature("director") carto::NMLModelLODTreeEventListener;

%include "layers/NMLModelLODTreeEventListener.h"

#endif

#endif
