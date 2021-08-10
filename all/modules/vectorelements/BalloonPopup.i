#ifndef _BALLOONPOPUP_I
#define _BALLOONPOPUP_I

%module BalloonPopup

!proxy_imports(carto::BalloonPopup, core.MapPos, core.ScreenPos, graphics.Bitmap, geometry.Geometry, styles.BalloonPopupStyle, ui.ClickInfo, vectorelements.BalloonPopupButton, vectorelements.BalloonPopupEventListener, vectorelements.Popup)
!java_imports(carto::BalloonPopup, com.carto.ui.ClickType)

%{
#include "vectorelements/BalloonPopup.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/ScreenPos.i"
%import "graphics/Bitmap.i"
%import "styles/BalloonPopupStyle.i"
%import "vectorelements/BalloonPopupEventListener.i"
%import "vectorelements/Popup.i"

!polymorphic_shared_ptr(carto::BalloonPopup, vectorelements.BalloonPopup)

%attributestring(carto::BalloonPopup, std::string, Title, getTitle, setTitle)
%attributestring(carto::BalloonPopup, std::string, Description, getDescription, setDescription)
%csmethodmodifiers carto::BalloonPopup::Style "public new";
!attributestring_polymorphic(carto::BalloonPopup, styles.BalloonPopupStyle, Style, getStyle, setStyle)
!attributestring_polymorphic(carto::BalloonPopup, vectorelements.BalloonPopupEventListener, BalloonPopupEventListener, getBalloonPopupEventListener, setBalloonPopupEventListener)
%std_exceptions(carto::BalloonPopup::BalloonPopup)
%std_exceptions(carto::BalloonPopup::setStyle)
%std_exceptions(carto::BalloonPopup::addButton)
%std_exceptions(carto::BalloonPopup::removeButton)

%include "vectorelements/BalloonPopup.h"

#endif
