#ifndef _STYLEBUILDER_I
#define _STYLEBUILDER_I

%module StyleBuilder

!proxy_imports(carto::StyleBuilder, graphics.Color)

%{
#include "styles/StyleBuilder.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"

!polymorphic_shared_ptr(carto::StyleBuilder, styles.StyleBuilder)

%attributeval(carto::StyleBuilder, carto::Color, Color, getColor, setColor)
%rename(clone) carto::StyleBuilder::operator=;
!standard_equals(carto::StyleBuilder);

%include "styles/StyleBuilder.h"

#endif
