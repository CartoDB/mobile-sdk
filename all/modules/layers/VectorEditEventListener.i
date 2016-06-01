#ifndef _VECTOREDITEVENTLISTENER_I
#define _VECTOREDITEVENTLISTENER_I

%module(directors="1") VectorEditEventListener

#ifdef _CARTO_EDITABLE_SUPPORT

!proxy_imports(carto::VectorEditEventListener, core.ScreenPos, geometry.Geometry, vectorelements.VectorElement, styles.PointStyle, ui.VectorElementDragInfo)

%{
#include "layers/VectorEditEventListener.h"
#include "ui/VectorElementDragInfo.h"
#include "core/ScreenPos.h"
#include "geometry/Geometry.h"
#include "vectorelements/VectorElement.h"
#include "styles/PointStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>

%import "core/ScreenPos.i"
%import "geometry/Geometry.i"
%import "vectorelements/VectorElement.i"
%import "styles/PointStyle.i"
%import "ui/VectorElementDragInfo.i"

!polymorphic_shared_ptr(carto::VectorEditEventListener, ui.VectorEditEventListener)

%feature("director") carto::VectorEditEventListener;

%include "layers/VectorEditEventListener.h"

#endif

#endif
