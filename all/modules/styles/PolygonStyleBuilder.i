#ifndef _POLYGONSTYLEBUILDER_I
#define _POLYGONSTYLEBUILDER_I

%module PolygonStyleBuilder

!proxy_imports(carto::PolygonStyleBuilder, styles.PolygonStyle, styles.StyleBuilder)

%{
#include "styles/PolygonStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/PolygonStyle.i"
%import "styles/StyleBuilder.i"

!polymorphic_shared_ptr(carto::PolygonStyleBuilder, styles.PolygonStyleBuilder)

%attributestring(carto::PolygonStyleBuilder, std::shared_ptr<carto::LineStyle>, LineStyle, getLineStyle, setLineStyle)

%include "styles/PolygonStyleBuilder.h"

#endif
