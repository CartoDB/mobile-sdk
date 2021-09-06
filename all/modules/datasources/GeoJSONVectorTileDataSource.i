#ifndef _GEOJSONVECTORTILEDATASOURCE_I
#define _GEOJSONVECTORTILEDATASOURCE_I

%module(directors="1") GeoJSONVectorTileDataSource

!proxy_imports(carto::GeoJSONVectorTileDataSource, core.MapTile, core.MapBounds, core.Variant, datasources.TileDataSource, datasources.components.TileData, geometry.FeatureCollection, projections.Projection)

%{
#include "datasources/GeoJSONVectorTileDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/MapTile.i"
%import "core/Variant.i"
%import "geometry/FeatureCollection.i"
%import "datasources/TileDataSource.i"
%import "datasources/components/TileData.i"
%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::GeoJSONVectorTileDataSource, datasources.GeoJSONVectorTileDataSource)

%attribute(carto::GeoJSONVectorTileDataSource, float, SimplifyTolerance, getSimplifyTolerance, setSimplifyTolerance)
%std_io_exceptions(carto::GeoJSONVectorTileDataSource::createLayer)
%std_io_exceptions(carto::GeoJSONVectorTileDataSource::setLayerGeoJSON)
%std_io_exceptions(carto::GeoJSONVectorTileDataSource::setLayerFeatureCollection)

%feature("director") carto::GeoJSONVectorTileDataSource;

%include "datasources/GeoJSONVectorTileDataSource.h"

#endif
