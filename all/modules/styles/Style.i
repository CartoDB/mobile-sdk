#ifndef _STYLE_I
#define _STYLE_I

%module Style

!proxy_imports(carto::Style, graphics.Color)

%{
#include "styles/Style.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"

!polymorphic_shared_ptr(carto::Style, styles.Style)

%attributeval(carto::Style, carto::Color, Color, getColor)
!standard_equals(carto::Style);

%include "styles/Style.h"

#endif
