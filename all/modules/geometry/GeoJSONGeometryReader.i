#ifndef _GEOJSONGEOMETRYREADER_I
#define _GEOJSONGEOMETRYREADER_I

%module GeoJSONGeometryReader

!proxy_imports(carto::GeoJSONGeometryReader, geometry.Feature, geometry.FeatureCollection, geometry.Geometry, projections.Projection)

%{
#include "geometry/GeoJSONGeometryReader.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geometry/Feature.i"
%import "geometry/FeatureCollection.i"
%import "geometry/Geometry.i"
%import "projections/Projection.i"

!attributestring_polymorphic(carto::GeoJSONGeometryReader, projections.Projection, TargetProjection, getTargetProjection, setTargetProjection)
%std_exceptions(carto::GeoJSONGeometryReader::readGeometry)
%std_exceptions(carto::GeoJSONGeometryReader::readFeature)
%std_exceptions(carto::GeoJSONGeometryReader::readFeatureCollection)

%include "geometry/GeoJSONGeometryReader.h"

#endif
