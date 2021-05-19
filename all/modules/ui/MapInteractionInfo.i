#ifndef _MAPINTERACTIONINFO_I
#define _MAPINTERACTIONINFO_I

%module MapInteractionInfo

!proxy_imports(carto::MapInteractionInfo)

%{
#include "ui/MapInteractionInfo.h"
#include <memory>
%}

%import <std_shared_ptr.i>
%include <cartoswig.i>

!shared_ptr(carto::MapInteractionInfo, ui.MapInteractionInfo)

%attribute(carto::MapInteractionInfo, bool, PanAction, isPanAction)
%attribute(carto::MapInteractionInfo, bool, ZoomAction, isZoomAction)
%attribute(carto::MapInteractionInfo, bool, RotateAction, isRotateAction)
%attribute(carto::MapInteractionInfo, bool, TiltAction, isTiltAction)
%ignore carto::MapInteractionInfo::MapInteractionInfo;
!standard_equals(carto::MapInteractionInfo);

%include "ui/MapInteractionInfo.h"

#endif
