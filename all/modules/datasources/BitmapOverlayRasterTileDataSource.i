#ifndef _BITMAPOVERLAYRASTERTILEDATASOURCE_I
#define _BITMAPOVERLAYRASTERTILEDATASOURCE_I

%module(directors="1") BitmapOverlayRasterTileDataSource

!proxy_imports(carto::BitmapOverlayRasterTileDataSource, core.MapPos, core.MapPosVector, core.MapBounds, core.ScreenPos, core.ScreenPosVector, core.ScreenPosVector, core.MapTile, core.StringMap, datasources.TileDataSource, datasources.components.TileData, projections.Projection, graphics.Bitmap)

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
%import "datasources/TileDataSource.i"
%import "datasources/components/TileData.i"
%import "projections/Projection.i"
%import "graphics/Bitmap.i"

!polymorphic_shared_ptr(carto::BitmapOverlayRasterTileDataSource, datasources.BitmapOverlayRasterTileDataSource)

%attributeval(carto::BitmapOverlayRasterTileDataSource, carto::MapBounds, DataExtent, getDataExtent)
%std_exceptions(carto::BitmapOverlayRasterTileDataSource::BitmapOverlayRasterTileDataSource)

%feature("director") carto::BitmapOverlayRasterTileDataSource;

%include "datasources/BitmapOverlayRasterTileDataSource.h"

#endif
