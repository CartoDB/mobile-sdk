#ifndef _WKTGEOMETRYWRITER_I
#define _WKTGEOMETRYWRITER_I

%module WKTGeometryWriter

#ifdef _CARTO_WKBT_SUPPORT

!proxy_imports(carto::WKTGeometryWriter, geometry.Geometry)

%{
#include "geometry/WKTGeometryWriter.h"
#include "geometry/Geometry.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"

%attribute(carto::WKTGeometryWriter, bool, Z, getZ, setZ)
%std_exceptions(carto::WKTGeometryWriter::writeGeometry)

%include "geometry/WKTGeometryWriter.h"

#endif

#endif
