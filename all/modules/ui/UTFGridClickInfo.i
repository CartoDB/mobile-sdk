#ifndef _UTFGRIDCLICKINFO_I
#define _UTFGRIDCLICKINFO_I

%module UTFGridClickInfo

!proxy_imports(carto::UTFGridClickInfo, core.MapPos, core.Variant, layers.Layer, ui.ClickInfo)

%{
#include "ui/UTFGridClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_map.i>
%include <cartoswig.i>

%import "ui/ClickInfo.i"
%import "core/MapPos.i"
%import "core/Variant.i"
%import "layers/Layer.i"

!shared_ptr(carto::UTFGridClickInfo, ui.UTFGridClickInfo)

%attribute(carto::UTFGridClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attributeval(carto::UTFGridClickInfo, carto::ClickInfo, ClickInfo, getClickInfo)
%attributeval(carto::UTFGridClickInfo, carto::MapPos, ClickPos, getClickPos)
%attributeval(carto::UTFGridClickInfo, carto::Variant, ElementInfo, getElementInfo)
!attributestring_polymorphic(carto::UTFGridClickInfo, layers.Layer, Layer, getLayer)
%ignore carto::UTFGridClickInfo::UTFGridClickInfo;
!standard_equals(carto::UTFGridClickInfo);

%include "ui/UTFGridClickInfo.h"

#endif
