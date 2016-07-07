#ifndef _WKBGEOMETRYREADER_I
#define _WKBGEOMETRYREADER_I

%module WKBGeometryReader

#ifdef _CARTO_WKBT_SUPPORT

!proxy_imports(carto::WKBGeometryReader, core.BinaryData, geometry.Geometry)

%{
#include "geometry/WKBGeometryReader.h"
#include "geometry/Geometry.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "geometry/Geometry.i"

%std_exceptions(carto::WKBGeometryReader::readGeometry)

%include "geometry/WKBGeometryReader.h"

#endif

#endif
