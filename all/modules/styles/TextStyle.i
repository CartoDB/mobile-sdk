#ifndef _TEXTSTYLE_I
#define _TEXTSTYLE_I

%module TextStyle

!proxy_imports(carto::TextStyle, graphics.Color, styles.LabelStyle)

%{
#include "styles/TextStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/LabelStyle.i"

!polymorphic_shared_ptr(carto::TextStyle, styles.TextStyle)

%attributeval(carto::TextStyle, carto::Color, FontColor, getFontColor)
%attributestring(carto::TextStyle, std::string, FontName, getFontName)
%attributestring(carto::TextStyle, std::string, TextField, getTextField)
%attribute(carto::TextStyle, int, FontSize, getFontSize)
%attributeval(carto::TextStyle, carto::Color, StrokeColor, getStrokeColor)
%attribute(carto::TextStyle, float, StrokeWidth, getStrokeWidth)

%include "styles/TextStyle.h"

#endif
