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

%import "styles/Style.i"

!polymorphic_shared_ptr(carto::Polygon3DStyle, styles.Polygon3DStyle)

%include "styles/Polygon3DStyle.h"

#endif
