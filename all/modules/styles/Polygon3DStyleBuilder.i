#ifndef _POLYGON3DSTYLEBUILDER_I
#define _POLYGON3DSTYLEBUILDER_I

%module Polygon3DStyleBuilder

!proxy_imports(carto::Polygon3DStyleBuilder, graphics.Color, styles.Polygon3DStyle, styles.StyleBuilder)

%{
#include "styles/Polygon3DStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"
%import "styles/Polygon3DStyle.i"
%import "styles/StyleBuilder.i"

!polymorphic_shared_ptr(carto::Polygon3DStyleBuilder, styles.Polygon3DStyleBuilder)

%include "styles/Polygon3DStyleBuilder.h"

#endif
