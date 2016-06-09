#ifndef _GDALRASTERTILEDATASOURCE_I
#define _GDALRASTERTILEDATASOURCE_I

%module(directors="1") GDALRasterTileDataSource

#ifdef _CARTO_GDAL_SUPPORT

!proxy_imports(carto::GDALRasterTileDataSource, core.MapTile, core.MapBounds, datasources.components.TileData)

%{
#include "datasources/GDALRasterTileDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapTile.i"
%import "core/MapBounds.i"
%import "datasources/TileDataSource.i"
%import "datasources/components/TileData.i"

!polymorphic_shared_ptr(carto::GDALRasterTileDataSource, datasources.GDALRasterTileDataSource)

%feature("director") carto::GDALRasterTileDataSource;

%attributeval(carto::GDALRasterTileDataSource, carto::MapBounds, DataExtent, getDataExtent)

%include "datasources/GDALRasterTileDataSource.h"

#endif

#endif
