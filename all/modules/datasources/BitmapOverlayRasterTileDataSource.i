#ifndef _BITMAPOVERLAYRASTERTILEDATASOURCE_I
#define _BITMAPOVERLAYRASTERTILEDATASOURCE_I

%module(directors="1") BitmapOverlayRasterTileDataSource

!proxy_imports(carto::BitmapOverlayRasterTileDataSource, core.MapPos, core.MapPosVector, core.MapBounds, core.ScreenPos, core.ScreenPosVector, core.ScreenPosVector, core.MapTile, core.StringMap, core.TileData, datasources.TileDataSource, projections.Projection, graphics.Bitmap)

%{
#include "datasources/BitmapOverlayRasterTileDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/MapBounds.i"
%import "core/ScreenPos.i"
%import "core/MapTile.i"
%import "core/TileData.i"
%import "datasources/TileDataSource.i"
%import "projections/Projection.i"
%import "graphics/Bitmap.i"

!polymorphic_shared_ptr(carto::BitmapOverlayRasterTileDataSource, datasources.BitmapOverlayRasterTileDataSource)

%feature("director") carto::BitmapOverlayRasterTileDataSource;

%attributeval(carto::BitmapOverlayRasterTileDataSource, carto::MapBounds, DataExtent, getDataExtent)

%include "datasources/BitmapOverlayRasterTileDataSource.h"

#endif
