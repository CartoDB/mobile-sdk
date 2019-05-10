#ifndef _BALLOONPOPUPBUTTONCLICKINFO_I
#define _BALLOONPOPUPBUTTONCLICKINFO_I

%module BalloonPopupButtonClickInfo

!proxy_imports(carto::BalloonPopupButtonClickInfo, vectorelements.BalloonPopup, vectorelements.BalloonPopupButton, ui.ClickType)

%{
#include "ui/BalloonPopupButtonClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/ClickType.i"
%import "vectorelements/BalloonPopup.i"
%import "vectorelements/BalloonPopupButton.i"

!shared_ptr(carto::BalloonPopupButtonClickInfo, ui.BalloonPopupButtonClickInfo)

%attribute(carto::BalloonPopupButtonClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
!attributestring_polymorphic(carto::BalloonPopupButtonClickInfo, vectorelements.BalloonPopupButton, Button, getButton)
!attributestring_polymorphic(carto::BalloonPopupButtonClickInfo, vectorelements.BalloonPopup, BalloonPopup, getBalloonPopup)
%ignore carto::BalloonPopupButtonClickInfo::BalloonPopupButtonClickInfo;
!standard_equals(carto::BalloonPopupButtonClickInfo);

%include "ui/BalloonPopupButtonClickInfo.h"

#endif
