#ifndef _CARTOCSSPARSER_I
#define _CARTOCSSPARSER_I

%module CartoCSSParser

!proxy_imports(carto::CartoCSSParser, core.BinaryData, styles.PointStyleBuilder, styles.LineStyleBuilder, styles.PolygonStyleBuilder, styles.MarkerStyleBuilder)

%{
#include "styles/PointStyleBuilder.h"
#include "styles/LineStyleBuilder.h"
#include "styles/PolygonStyleBuilder.h"
#include "styles/MarkerStyleBuilder.h"
#include "styles/CartoCSSParser.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "styles/PointStyleBuilder.i"
%import "styles/LineStyleBuilder.i"
%import "styles/PolygonStyleBuilder.i"
%import "styles/MarkerStyleBuilder.i"

%include "styles/CartoCSSParser.h"

#endif
