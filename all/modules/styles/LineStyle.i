#ifndef _LINESTYLE_I
#define _LINESTYLE_I

%module LineStyle

!proxy_imports(carto::LineStyle, graphics.Bitmap, graphics.Color, styles.Style)

%{
#include "styles/LineStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"
%import "styles/Style.i"

!polymorphic_shared_ptr(carto::LineStyle, styles.LineStyle)

%attribute(carto::LineStyle, float, Width, getWidth)
%attribute(carto::LineStyle, float, ClickWidth, getClickWidth)
%attribute(carto::LineStyle, float, StretchFactor, getStretchFactor)
%attribute(carto::LineStyle, carto::LineJointType::LineJointType, LineJointType, getLineJointType)
%attribute(carto::LineStyle, carto::LineEndType::LineEndType, LineEndType, getLineEndType)
%attributestring(carto::LineStyle, std::shared_ptr<carto::Bitmap>, Bitmap, getBitmap)

%include "styles/LineStyle.h"

#endif
