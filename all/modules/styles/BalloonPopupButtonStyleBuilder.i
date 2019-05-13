#ifndef _BALLOONPOPUPBUTTONSTYLEBUILDER_I
#define _BALLOONPOPUPBUTTONSTYLEBUILDER_I

%module BalloonPopupButtonStyleBuilder

!proxy_imports(carto::BalloonPopupButtonStyleBuilder, graphics.Color, styles.StyleBuilder, styles.BalloonPopupButtonStyle, styles.BalloonPopupMargins)

%{
#include "styles/BalloonPopupButtonStyleBuilder.h"
#include "styles/BalloonPopupStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"
%import "styles/StyleBuilder.i"
%import "styles/BalloonPopupButtonStyle.i"
%import "styles/BalloonPopupStyle.i"

!polymorphic_shared_ptr(carto::BalloonPopupButtonStyleBuilder, styles.BalloonPopupButtonStyleBuilder)

%attribute(carto::BalloonPopupButtonStyleBuilder, int, ButtonWidth, getButtonWidth, setButtonWidth)
%attribute(carto::BalloonPopupButtonStyleBuilder, int, CornerRadius, getCornerRadius, setCornerRadius)
%attributeval(carto::BalloonPopupButtonStyleBuilder, carto::Color, TextColor, getTextColor, setTextColor)
%attributestring(carto::BalloonPopupButtonStyleBuilder, std::string, TextFontName, getTextFontName, setTextFontName)
%attribute(carto::BalloonPopupButtonStyleBuilder, int, TextFontSize, getTextFontSize, setTextFontSize)
%attributeval(carto::BalloonPopupButtonStyleBuilder, carto::BalloonPopupMargins, TextMargins, getTextMargins, setTextMargins)
%attributeval(carto::BalloonPopupButtonStyleBuilder, carto::Color, StrokeColor, getStrokeColor, setStrokeColor)
%attribute(carto::BalloonPopupButtonStyleBuilder, int, StrokeWidth, getStrokeWidth, setStrokeWidth)
%csmethodmodifiers carto::BalloonPopupButtonStyleBuilder::buildStyle "public new";

%include "styles/BalloonPopupButtonStyleBuilder.h"

#endif
