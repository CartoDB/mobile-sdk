#ifndef _BASEMAPVIEW_I
#define _BASEMAPVIEW_I

%module BaseMapView

!proxy_imports(carto::BaseMapView, core.MapPos, core.MapVec, core.MapBounds, core.ScreenPos, core.ScreenBounds, components.Options, components.Layers, components.LicenseManagerListener, renderers.MapRenderer, renderers.RedrawRequestListener, ui.MapEventListener)
!java_imports(carto::BaseMapView, com.carto.components.ProjectionMode)

%{
#include "ui/BaseMapView.h"
#include "core/MapPos.h"
#include "core/ScreenPos.h"
%}

%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/MapBounds.i"
%import "core/ScreenPos.i"
%import "core/ScreenBounds.i"
%import "core/MapVec.i"
%import "components/Options.i"
%import "components/Layers.i"
%import "components/LicenseManagerListener.i"
%import "renderers/MapRenderer.i"
%import "renderers/RedrawRequestListener.i"
%import "ui/MapEventListener.i"

%include "ui/BaseMapView.h"

#endif
