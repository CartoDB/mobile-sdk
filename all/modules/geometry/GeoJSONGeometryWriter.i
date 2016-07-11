#ifndef _GEOJSONGEOMETRYWRITER_I
#define _GEOJSONGEOMETRYWRITER_I

%module GeoJSONGeometryWriter

!proxy_imports(carto::GeoJSONGeometryWriter, geometry.Feature, geometry.FeatureCollection, geometry.Geometry, projections.Projection)

%{
#include "geometry/GeoJSONGeometryWriter.h"
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

!attributestring_polymorphic(carto::GeoJSONGeometryWriter, projections.Projection, SourceProjection, getSourceProjection, setSourceProjection)
%attribute(carto::GeoJSONGeometryWriter, bool, Z, getZ, setZ)
%std_exceptions(carto::GeoJSONGeometryWriter::writeGeometry)
%std_exceptions(carto::GeoJSONGeometryWriter::writeFeature)
%std_exceptions(carto::GeoJSONGeometryWriter::writeFeatureCollection)

%include "geometry/GeoJSONGeometryWriter.h"

#endif
