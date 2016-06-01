#ifndef _POINTSTYLE_I
#define _POINTSTYLE_I

%module PointStyle

!proxy_imports(carto::PointStyle, graphics.Bitmap, graphics.Color, styles.Style)

%{
#include "styles/PointStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"
%import "styles/Style.i"

!polymorphic_shared_ptr(carto::PointStyle, styles.PointStyle)

%attribute(carto::PointStyle, float, Size, getSize)
%attribute(carto::PointStyle, float, ClickSize, getClickSize)
%attributestring(carto::PointStyle, std::shared_ptr<carto::Bitmap>, Bitmap, getBitmap)

%include "styles/PointStyle.h"

#endif
