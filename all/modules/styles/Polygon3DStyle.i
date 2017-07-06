#ifndef _POLYGON3DSTYLE_I
#define _POLYGON3DSTYLE_I

%module Polygon3DStyle

!proxy_imports(carto::Polygon3DStyle, graphics.Color, styles.Style)

%{
#include "styles/Polygon3DStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"
%import "styles/Style.i"

!polymorphic_shared_ptr(carto::Polygon3DStyle, styles.Polygon3DStyle)

%attributeval(carto::Polygon3DStyle, carto::Color, Color, getSideColor)
%ignore carto::Polygon3DStyle::Polygon3DStyle;

%include "styles/Polygon3DStyle.h"

#endif
