#ifndef _BALLOONPOPUP_I
#define _BALLOONPOPUP_I

%module BalloonPopup

!proxy_imports(carto::BalloonPopup, core.MapPos, core.ScreenPos, graphics.Bitmap, geometry.Geometry, styles.BalloonPopupStyle, vectorelements.Popup)

%{
#include "vectorelements/BalloonPopup.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/ScreenPos.i"
%import "graphics/Bitmap.i"
%import "styles/BalloonPopupStyle.i"
%import "vectorelements/Popup.i"

!polymorphic_shared_ptr(carto::BalloonPopup, vectorelements.BalloonPopup)

%attributestring(carto::BalloonPopup, std::string, Title, getTitle, setTitle)
%attributestring(carto::BalloonPopup, std::string, Description, getDescription, setDescription)
%csmethodmodifiers carto::BalloonPopup::Style "public new";
!attributestring_polymorphic(carto::BalloonPopup, styles.BalloonPopupStyle, Style, getStyle, setStyle)
%std_exceptions(carto::BalloonPopup::BalloonPopup)
%std_exceptions(carto::BalloonPopup::setStyle)

%include "vectorelements/BalloonPopup.h"

#endif
