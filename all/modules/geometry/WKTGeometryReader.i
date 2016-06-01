#ifndef _WKTGEOMETRYREADER_I
#define _WKTGEOMETRYREADER_I

%module WKTGeometryReader

#ifdef _CARTO_WKBT_SUPPORT

!proxy_imports(carto::WKTGeometryReader, geometry.Geometry)

%{
#include "geometry/WKTGeometryReader.h"
#include "geometry/Geometry.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"

%include "geometry/WKTGeometryReader.h"

#endif

#endif
