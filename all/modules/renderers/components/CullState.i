#ifndef _CULLSTATE_I
#define _CULLSTATE_I

%module CullState

!proxy_imports(carto::CullState, core.MapEnvelope, core.MapPos, graphics.ViewState, projections.Projection)

%{
#include "components/Exceptions.h"
#include "renderers/components/CullState.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapEnvelope.i"
%import "core/MapPos.i"
%import "graphics/ViewState.i"
%import "projections/Projection.i"

!shared_ptr(carto::CullState, renderers.components.CullState)

%attributeval(carto::CullState, carto::ViewState, ViewState, getViewState)
%std_exceptions(carto::CullState::getProjectionEnvelope)
%ignore carto::CullState::getEnvelope;
!standard_equals(carto::CullState);

%include "renderers/components/CullState.h"

#endif
