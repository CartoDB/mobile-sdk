#ifndef _UTFGRIDCLICKINFO_I
#define _UTFGRIDCLICKINFO_I

%module UTFGridClickInfo

!proxy_imports(carto::UTFGridClickInfo, core.MapPos, core.StringMap, layers.Layer, ui.ClickType)

%{
#include "ui/UTFGridClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_map.i>
%include <cartoswig.i>

// TODO: fix this, currently this fails to generate constructors for RasterTileLayer/VectorTileLayer
//%import "ui/ClickType.i"
%import "core/MapPos.i"
%import "core/StringMap.i"
%import "layers/Layer.i"

!shared_ptr(carto::UTFGridClickInfo, ui.UTFGridClickInfo)

// TODO: fix this, currently this fails to generate constructors for RasterTileLayer/VectorTileLayer
//%attribute(carto::UTFGridClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attributeval(carto::UTFGridClickInfo, carto::MapPos, ClickPos, getClickPos)
%attributeval(carto::UTFGridClickInfo, %arg(std::map<std::string, std::string>), ElementInfo, getElementInfo)
!attributestring_polymorphic(carto::UTFGridClickInfo, layers.Layer, Layer, getLayer)
// TODO: fix this, currently this fails to generate constructors for RasterTileLayer/VectorTileLayer
%ignore carto::UTFGridClickInfo::UTFGridClickInfo;
%ignore carto::UTFGridClickInfo::getClickType;
!standard_equals(carto::UTFGridClickInfo);

%include "ui/UTFGridClickInfo.h"

#endif
