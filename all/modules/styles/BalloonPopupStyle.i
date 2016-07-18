#ifndef _BALLOONPOPUPSTYLE_I
#define _BALLOONPOPUPSTYLE_I

%module BalloonPopupStyle

!proxy_imports(carto::BalloonPopupStyle, graphics.Color, graphics.Bitmap, styles.PopupStyle)

%{
#include "styles/BalloonPopupStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/PopupStyle.i"

!polymorphic_shared_ptr(carto::BalloonPopupStyle, styles.BalloonPopupStyle)

%attributeval(carto::BalloonPopupStyle, carto::Color, BackgroundColor, getBackgroundColor)
%attribute(carto::BalloonPopupStyle, int, CornerRadius, getCornerRadius)
%attributeval(carto::BalloonPopupStyle, carto::Color, LeftColor, getLeftColor)
%attributestring(carto::BalloonPopupStyle, std::shared_ptr<carto::Bitmap>, LeftImage, getLeftImage)
%attributeval(carto::BalloonPopupStyle, carto::BalloonPopupMargins, LeftMargins, getLeftMargins)
%attributeval(carto::BalloonPopupStyle, carto::Color, RightColor, getRightColor)
%attributestring(carto::BalloonPopupStyle, std::shared_ptr<carto::Bitmap>, RightImage, getRightImage)
%attributeval(carto::BalloonPopupStyle, carto::BalloonPopupMargins, RightMargins, getRightMargins)
%attributeval(carto::BalloonPopupStyle, carto::Color, TitleColor, getTitleColor)
%attributestring(carto::BalloonPopupStyle, std::string, TitleFontName, getTitleFontName)
%attributestring(carto::BalloonPopupStyle, std::string, TitleField, getTitleField)
%attribute(carto::BalloonPopupStyle, int, TitleFontSize, getTitleFontSize)
%attributeval(carto::BalloonPopupStyle, carto::BalloonPopupMargins, TitleMargins, getTitleMargins)
%attribute(carto::BalloonPopupStyle, bool, TitleWrap, isTitleWrap)
%attributeval(carto::BalloonPopupStyle, carto::Color, DescriptionColor, getDescriptionColor)
%attributestring(carto::BalloonPopupStyle, std::string, DescriptionFontName, getDescriptionFontName)
%attributestring(carto::BalloonPopupStyle, std::string, DescriptionField, getDescriptionField)
%attribute(carto::BalloonPopupStyle, int, DescriptionFontSize, getDescriptionFontSize)
%attributeval(carto::BalloonPopupStyle, carto::BalloonPopupMargins, DescriptionMargins, getDescriptionMargins)
%attribute(carto::BalloonPopupStyle, bool, DescriptionWrap, isDescriptionWrap)
%attributeval(carto::BalloonPopupStyle, carto::Color, StrokeColor, getStrokeColor)
%attribute(carto::BalloonPopupStyle, int, StrokeWidth, getStrokeWidth)
%attribute(carto::BalloonPopupStyle, int, TriangleWidth, getTriangleWidth)
%attribute(carto::BalloonPopupStyle, int, TriangleHeight, getTriangleHeight)

%include "styles/BalloonPopupStyle.h"

#endif
