#ifndef _PACKAGEMANAGERTILEDATASOURCE_I
#define _PACKAGEMANAGERTILEDATASOURCE_I

%module(directors="1") PackageManagerTileDataSource

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

!proxy_imports(carto::PackageManagerTileDataSource, core.MapTile, core.StringMap, datasources.TileDataSource, datasources.components.TileData, packagemanager.PackageManager)

%{
#include "datasources/PackageManagerTileDataSource.h"
#include "packagemanager/PackageManager.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "packagemanager/PackageManager.i"

!polymorphic_shared_ptr(carto::PackageManagerTileDataSource, datasources.PackageManagerTileDataSource)

%feature("director") carto::PackageManagerTileDataSource;

!attributestring_polymorphic(carto::PackageManagerTileDataSource, packagemanager.PackageManager, PackageManager, getPackageManager)

%include "datasources/PackageManagerTileDataSource.h"

#endif

#endif
