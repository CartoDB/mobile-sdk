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

!value_type(carto::TextMargins, styles.TextMargins)

%attribute(carto::TextMargins, int, Left, getLeft)
%attribute(carto::TextMargins, int, Right, getRight)
%attribute(carto::TextMargins, int, Top, getTop)
%attribute(carto::TextMargins, int, Bottom, getBottom)

!polymorphic_shared_ptr(carto::TextStyle, styles.TextStyle)

%attributeval(carto::TextStyle, carto::Color, FontColor, getFontColor)
%attributestring(carto::TextStyle, std::string, FontName, getFontName)
%attributestring(carto::TextStyle, std::string, TextField, getTextField)
%attribute(carto::TextStyle, float, FontSize, getFontSize)
%attribute(carto::TextStyle, bool, BreakLines, isBreakLines)
%attributeval(carto::TextStyle, carto::TextMargins, TextMargins, getTextMargins)
%attributeval(carto::TextStyle, carto::Color, StrokeColor, getStrokeColor)
%attribute(carto::TextStyle, float, StrokeWidth, getStrokeWidth)
%attributeval(carto::TextStyle, carto::Color, BorderColor, getBorderColor)
%attribute(carto::TextStyle, float, BorderWidth, getBorderWidth)
%attributeval(carto::TextStyle, carto::Color, BackgroundColor, getBackgroundColor)
%ignore carto::TextStyle::TextStyle;

%include "styles/TextStyle.h"

#endif
