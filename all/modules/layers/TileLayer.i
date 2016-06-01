#ifndef _TILELAYER_I
#define _TILELAYER_I

#pragma SWIG nowarn=402

%module TileLayer

!proxy_imports(carto::TileLayer, core.MapPos, core.MapTile, core.MapBounds, datasources.TileDataSource, layers.TileLoadListener, layers.Layer)

%{
#include "layers/TileLayer.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "layers/Layer.i"
%import "layers/TileLoadListener.i"

!polymorphic_shared_ptr(carto::TileLayer, layers.TileLayer)

%attribute(carto::TileLayer, int, FrameNr, getFrameNr, setFrameNr)
%attribute(carto::TileLayer, bool, Preloading, isPreloading, setPreloading)
%attribute(carto::TileLayer, bool, SynchronizedRefresh, isSynchronizedRefresh, setSynchronizedRefresh)
%attribute(carto::TileLayer, carto::TileSubstitutionPolicy::TileSubstitutionPolicy, TileSubstitutionPolicy, getTileSubstitutionPolicy, setTileSubstitutionPolicy)
%attribute(carto::TileLayer, float, ZoomLevelBias, getZoomLevelBias, setZoomLevelBias)
!attributestring_polymorphic(carto::TileLayer, datasources.TileDataSource, DataSource, getDataSource)
!attributestring_polymorphic(carto::TileLayer, layers.TileLoadListener, TileLoadListener, getTileLoadListener, setTileLoadListener)
%ignore carto::TileLayer::FetchTaskBase;
%ignore carto::TileLayer::FetchingTiles;
%ignore carto::TileLayer::DataSourceListener;
%ignore carto::TileLayer::getMinZoom;
%ignore carto::TileLayer::getMaxZoom;

%include "layers/TileLayer.h"

#endif
