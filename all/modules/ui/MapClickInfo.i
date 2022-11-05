#ifndef _MAPCLICKINFO_I
#define _MAPCLICKINFO_I

%module MapClickInfo

!proxy_imports(carto::MapClickInfo, core.MapPos, ui.ClickInfo)

%{
#include "ui/MapClickInfo.h"
#include <memory>
%}

%import <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/ClickInfo.i"
%import "core/MapPos.i"

!shared_ptr(carto::MapClickInfo, ui.MapClickInfo)

%attribute(carto::MapClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attributeval(carto::MapClickInfo, carto::ClickInfo, ClickInfo, getClickInfo)
%attributeval(carto::MapClickInfo, carto::MapPos, ClickPos, getClickPos)
%ignore carto::MapClickInfo::MapClickInfo;
!standard_equals(carto::MapClickInfo);

%include "ui/MapClickInfo.h"

#endif
