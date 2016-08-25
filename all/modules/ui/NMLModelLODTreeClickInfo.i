#ifndef _NMLMODELLODTREECLICKINFO_I
#define _NMLMODELLODTREECLICKINFO_I

%module NMLModelLODTreeClickInfo

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

!proxy_imports(carto::NMLModelLODTreeClickInfo, core.MapPos, core.StringMap, layers.Layer, ui.ClickType)

%{
#include "ui/NMLModelLODTreeClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "ui/ClickType.i"
%import "core/MapPos.i"
%import "core/StringMap.i"
%import "layers/Layer.i"

!shared_ptr(carto::NMLModelLODTreeClickInfo, ui.NMLModelLODTreeClickInfo)

%attribute(carto::NMLModelLODTreeClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attributeval(carto::NMLModelLODTreeClickInfo, carto::MapPos, ClickPos, getClickPos)
%attributeval(carto::NMLModelLODTreeClickInfo, carto::MapPos, ElementClickPos, getElementClickPos)
%attributeval(carto::NMLModelLODTreeClickInfo, %arg(std::map<std::string, std::string>), MetaData, getMetaData)
!attributestring_polymorphic(carto::NMLModelLODTreeClickInfo, layers.Layer, Layer, getLayer)
!standard_equals(carto::NMLModelLODTreeClickInfo);

%include "ui/NMLModelLODTreeClickInfo.h"

#endif

#endif
