#ifndef _BALLOONPOPUPSTYLEBUILDER_I
#define _BALLOONPOPUPSTYLEBUILDER_I

%module BalloonPopupStyleBuilder

!proxy_imports(carto::BalloonPopupStyleBuilder, graphics.Color, graphics.Bitmap, styles.PopupStyleBuilder, styles.BalloonPopupStyle)

%{
#include "styles/BalloonPopupStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Color.i"
%import "graphics/Bitmap.i"
%import "styles/PopupStyleBuilder.i"
%import "styles/BalloonPopupStyle.i"

!polymorphic_shared_ptr(carto::BalloonPopupStyleBuilder, styles.BalloonPopupStyleBuilder)

%attribute(carto::BalloonPopupStyleBuilder, int, CornerRadius, getCornerRadius, setCornerRadius)
%attributeval(carto::BalloonPopupStyleBuilder, carto::Color, LeftColor, getLeftColor, setLeftColor)
%attributestring(carto::BalloonPopupStyleBuilder, std::shared_ptr<carto::Bitmap>, LeftImage, getLeftImage, setLeftImage)
%attributeval(carto::BalloonPopupStyleBuilder, carto::BalloonPopupMargins, LeftMargins, getLeftMargins, setLeftMargins)
%attributeval(carto::BalloonPopupStyleBuilder, carto::Color, RightColor, getRightColor, setRightColor)
%attributestring(carto::BalloonPopupStyleBuilder, std::shared_ptr<carto::Bitmap>, RightImage, getRightImage, setRightImage)
%attributeval(carto::BalloonPopupStyleBuilder, carto::BalloonPopupMargins, RightMargins, getRightMargins, setRightMargins)
%attributeval(carto::BalloonPopupStyleBuilder, carto::Color, TitleColor, getTitleColor, setTitleColor)
%attributestring(carto::BalloonPopupStyleBuilder, std::string, TitleFontName, getTitleFontName, setTitleFontName)
%attribute(carto::BalloonPopupStyleBuilder, int, TitleFontSize, getTitleFontSize, setTitleFontSize)
%attributeval(carto::BalloonPopupStyleBuilder, carto::BalloonPopupMargins, TitleMargins, getTitleMargins, setTitleMargins)
%attribute(carto::BalloonPopupStyleBuilder, bool, TitleWrap, isTitleWrap, setTitleWrap)
%attributeval(carto::BalloonPopupStyleBuilder, carto::Color, DescriptionColor, getDescriptionColor, setDescriptionColor)
%attributestring(carto::BalloonPopupStyleBuilder, std::string, DescriptionFontName, getDescriptionFontName, setDescriptionFontName)
%attribute(carto::BalloonPopupStyleBuilder, int, DescriptionFontSize, getDescriptionFontSize, setDescriptionFontSize)
%attributeval(carto::BalloonPopupStyleBuilder, carto::BalloonPopupMargins, DescriptionMargins, getDescriptionMargins, setDescriptionMargins)
%attribute(carto::BalloonPopupStyleBuilder, bool, DescriptionWrap, isDescriptionWrap, setDescriptionWrap)
%attributeval(carto::BalloonPopupStyleBuilder, carto::Color, StrokeColor, getStrokeColor, setStrokeColor)
%attribute(carto::BalloonPopupStyleBuilder, int, StrokeWidth, getStrokeWidth, setStrokeWidth)
%attribute(carto::BalloonPopupStyleBuilder, int, TriangleWidth, getTriangleWidth, setTriangleWidth)
%attribute(carto::BalloonPopupStyleBuilder, int, TriangleHeight, getTriangleHeight, setTriangleHeight)
%csmethodmodifiers carto::BalloonPopupStyleBuilder::buildStyle "public new";

%include "styles/BalloonPopupStyleBuilder.h"

#endif
