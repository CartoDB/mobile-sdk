#ifndef _TEXTSTYLEBUILDER_I
#define _TEXTSTYLEBUILDER_I

%module TextStyleBuilder

!proxy_imports(carto::TextStyleBuilder, graphics.Color, graphics.Bitmap, styles.LabelStyleBuilder, styles.TextStyle)

%{
#include "styles/TextStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/LabelStyleBuilder.i"
%import "styles/TextStyle.i"

!polymorphic_shared_ptr(carto::TextStyleBuilder, styles.TextStyleBuilder)

%attributestring(carto::TextStyleBuilder, std::string, FontName, getFontName, setFontName)
%attributestring(carto::TextStyleBuilder, std::string, TextField, getTextField, setTextField)
%attribute(carto::TextStyleBuilder, int, FontSize, getFontSize, setFontSize)
%attributeval(carto::TextStyleBuilder, carto::Color, StrokeColor, getStrokeColor, setStrokeColor)
%attribute(carto::TextStyleBuilder, float, StrokeWidth, getStrokeWidth, setStrokeWidth)
%csmethodmodifiers carto::TextStyleBuilder::buildStyle "public new";

%include "styles/TextStyleBuilder.h"

#endif
