#ifndef _GEOJSONGEOMETRYWRITER_I
#define _GEOJSONGEOMETRYWRITER_I

%module GeoJSONGeometryWriter

!proxy_imports(carto::GeoJSONGeometryWriter, geometry.Geometry, projections.Projection)

%{
#include "geometry/GeoJSONGeometryWriter.h"
#include "geometry/Geometry.h"
#include "projections/Projection.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"
%import "projections/Projection.i"

!attributestring_polymorphic(carto::GeoJSONGeometryWriter, projections.Projection, SourceProjection, getSourceProjection, setSourceProjection)
%attribute(carto::GeoJSONGeometryWriter, bool, Z, getZ, setZ)
%std_exceptions(carto::GeoJSONGeometryWriter::writeGeometry)

%include "geometry/GeoJSONGeometryWriter.h"

#endif
