#ifndef _POLYGONSTYLE_I
#define _POLYGONSTYLE_I

%module PolygonStyle

!proxy_imports(carto::PolygonStyle, graphics.Bitmap, graphics.Color, styles.LineStyle)

%{
#include "styles/PolygonStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/LineStyle.i"

!polymorphic_shared_ptr(carto::PolygonStyle, styles.PolygonStyle)

%attributestring(carto::PolygonStyle, std::shared_ptr<carto::LineStyle>, LineStyle, getLineStyle)
%attributestring(carto::PolygonStyle, std::shared_ptr<carto::Bitmap>, Bitmap, getBitmap)

%include "styles/PolygonStyle.h"

#endif
