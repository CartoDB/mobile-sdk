#ifndef _BALLOONPOPUPBUTTONSTYLE_I
#define _BALLOONPOPUPBUTTONSTYLE_I

%module BalloonPopupButtonStyle

!proxy_imports(carto::BalloonPopupButtonStyle, graphics.Color, styles.BalloonPopupStyle, styles.BalloonPopupMargins, styles.Style)

%{
#include "styles/BalloonPopupButtonStyle.h"
#include "styles/BalloonPopupStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/Style.i"
%import "styles/BalloonPopupStyle.i"

!polymorphic_shared_ptr(carto::BalloonPopupButtonStyle, styles.BalloonPopupButtonStyle)

%attributeval(carto::BalloonPopupButtonStyle, carto::Color, BackgroundColor, getBackgroundColor)
%attribute(carto::BalloonPopupButtonStyle, int, ButtonWidth, getButtonWidth)
%attribute(carto::BalloonPopupButtonStyle, int, CornerRadius, getCornerRadius)
%attributeval(carto::BalloonPopupButtonStyle, carto::Color, TextColor, getTextColor)
%attributestring(carto::BalloonPopupButtonStyle, std::string, TextFontName, getTextFontName)
%attribute(carto::BalloonPopupButtonStyle, int, TextFontSize, getTextFontSize)
%attributeval(carto::BalloonPopupButtonStyle, carto::BalloonPopupMargins, TextMargins, getTextMargins)
%attributeval(carto::BalloonPopupButtonStyle, carto::Color, StrokeColor, getStrokeColor)
%attribute(carto::BalloonPopupButtonStyle, int, StrokeWidth, getStrokeWidth)
%ignore carto::BalloonPopupButtonStyle::BalloonPopupButtonStyle;

%include "styles/BalloonPopupButtonStyle.h"

#endif
