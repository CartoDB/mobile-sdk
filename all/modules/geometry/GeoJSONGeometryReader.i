#ifndef _GEOJSONGEOMETRYREADER_I
#define _GEOJSONGEOMETRYREADER_I

%module GeoJSONGeometryReader

!proxy_imports(carto::GeoJSONGeometryReader, geometry.Geometry, projections.Projection)

%{
#include "geometry/GeoJSONGeometryReader.h"
#include "geometry/Geometry.h"
#include "projections/Projection.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"
%import "projections/Projection.i"

!attributestring_polymorphic(carto::GeoJSONGeometryReader, projections.Projection, TargetProjection, getTargetProjection, setTargetProjection)

%include "geometry/GeoJSONGeometryReader.h"

#endif
