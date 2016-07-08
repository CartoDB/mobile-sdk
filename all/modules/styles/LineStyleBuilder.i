#ifndef _LINESTYLEBUILDER_I
#define _LINESTYLEBUILDER_I

%module LineStyleBuilder

!proxy_imports(carto::LineStyleBuilder, graphics.Bitmap, styles.LineStyle, styles.StyleBuilder)

%{
#include "styles/LineStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/LineStyle.i"
%import "styles/StyleBuilder.i"

!polymorphic_shared_ptr(carto::LineStyleBuilder, styles.LineStyleBuilder)

%attribute(carto::LineStyleBuilder, float, Width, getWidth, setWidth)
%attribute(carto::LineStyleBuilder, float, ClickWidth, getClickWidth, setClickWidth)
%attribute(carto::LineStyleBuilder, float, StretchFactor, getStretchFactor, setStretchFactor)
%attribute(carto::LineStyleBuilder, carto::LineJoinType::LineJoinType, LineJoinType, getLineJoinType, setLineJoinType)
%attribute(carto::LineStyleBuilder, carto::LineEndType::LineEndType, LineEndType, getLineEndType, setLineEndType)
%attributestring(carto::LineStyleBuilder, std::shared_ptr<carto::Bitmap>, Bitmap, getBitmap, setBitmap)
%std_exceptions(carto::LineStyleBuilder::setBitmap)

%include "styles/LineStyleBuilder.h"

#endif
