#ifndef _WKBGEOMETRYWRITER_I
#define _WKBGEOMETRYWRITER_I

%module WKBGeometryWriter

#ifdef _CARTO_WKBT_SUPPORT

!proxy_imports(carto::WKBGeometryWriter, core.BinaryData, geometry.Geometry)

%{
#include "geometry/WKBGeometryWriter.h"
#include "geometry/Geometry.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "geometry/Geometry.i"

%attribute(carto::WKBGeometryWriter, bool, Z, getZ, setZ)
%attribute(carto::WKBGeometryWriter, bool, BigEndian, getBigEndian, setBigEndian)
%std_exceptions(carto::WKBGeometryWriter::writeGeometry)

%include "geometry/WKBGeometryWriter.h"

#endif

#endif
