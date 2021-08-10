#ifndef _RASTERTILECLICKINFO_I
#define _RASTERTILECLICKINFO_I

%module RasterTileClickInfo

!proxy_imports(carto::RasterTileClickInfo, core.MapPos, core.MapTile, graphics.Color, layers.Layer, ui.ClickInfo)

%{
#include "ui/RasterTileClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "ui/ClickInfo.i"
%import "core/MapPos.i"
%import "core/MapTile.i"
%import "graphics/Color.i"
%import "layers/Layer.i"

!shared_ptr(carto::RasterTileClickInfo, ui.RasterTileClickInfo)

%attribute(carto::RasterTileClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attributeval(carto::RasterTileClickInfo, carto::ClickInfo, ClickInfo, getClickInfo)
%attributeval(carto::RasterTileClickInfo, carto::MapPos, ClickPos, getClickPos)
%attributeval(carto::RasterTileClickInfo, carto::MapTile, MapTile, getMapTile)
%attributeval(carto::RasterTileClickInfo, carto::Color, NearestColor, getNearestColor)
%attributeval(carto::RasterTileClickInfo, carto::Color, InterpolatedColor, getInterpolatedColor)
!attributestring_polymorphic(carto::RasterTileClickInfo, layers.Layer, Layer, getLayer)
%ignore carto::RasterTileClickInfo::RasterTileClickInfo;
!standard_equals(carto::RasterTileClickInfo);

%include "ui/RasterTileClickInfo.h"

#endif
