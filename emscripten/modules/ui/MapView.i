#ifndef _MAPVIEW_I
#define _MAPVIEW_I

%module MAPVIEW

!proxy_imports(carto::MapView, ui.MapView)

%{
#include "ui/MapView.h"
#include "ui/BaseMapView.h"
#include "ui/MapEventListener.h"
#include "ui/MapRedrawRequestListener.h"
#include "ui/EmscriptenInput.h"
#include "components/Options.h"
#include "components/Layers.h"
#include "core/MapBounds.h"
#include "core/MapPos.h"
#include "core/MapVec.h"
#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "utils/Const.h"
#include "renderers/MapRenderer.h"
%}

%include <std_string.i>

%include "ui/MapView.h"

#endif
