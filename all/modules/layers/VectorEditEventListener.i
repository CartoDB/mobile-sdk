#ifndef _VECTOREDITEVENTLISTENER_I
#define _VECTOREDITEVENTLISTENER_I

%module(directors="1") VectorEditEventListener

#ifdef _CARTO_EDITABLE_SUPPORT

!proxy_imports(carto::VectorEditEventListener, core.ScreenPos, geometry.Geometry, vectorelements.VectorElement, styles.PointStyle, ui.VectorElementDragInfo)

%{
#include "layers/VectorEditEventListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>

%import "core/ScreenPos.i"
%import "geometry/Geometry.i"
%import "vectorelements/VectorElement.i"
%import "styles/PointStyle.i"
%import "ui/VectorElementDragInfo.i"

!polymorphic_shared_ptr(carto::VectorEditEventListener, layers.VectorEditEventListener)

%feature("director") carto::VectorEditEventListener;

%include "layers/VectorEditEventListener.h"

#endif

#endif
