#ifndef _BALLOONPOPUPBUTTONCLICKINFO_I
#define _BALLOONPOPUPBUTTONCLICKINFO_I

%module BalloonPopupButtonClickInfo

!proxy_imports(carto::BalloonPopupButtonClickInfo, ui.ClickType, vectorelements.BalloonPopupButton, vectorelements.VectorElement)

%{
#include "ui/BalloonPopupButtonClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/ClickType.i"
%import "vectorelements/BalloonPopupButton.i"
%import "vectorelements/VectorElement.i"

!shared_ptr(carto::BalloonPopupButtonClickInfo, ui.BalloonPopupButtonClickInfo)

%attribute(carto::BalloonPopupButtonClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
!attributestring_polymorphic(carto::BalloonPopupButtonClickInfo, vectorelements.BalloonPopupButton, Button, getButton)
!attributestring_polymorphic(carto::BalloonPopupButtonClickInfo, vectorelements.VectorElement, VectorElement, getVectorElement)
%ignore carto::BalloonPopupButtonClickInfo::BalloonPopupButtonClickInfo;
!standard_equals(carto::BalloonPopupButtonClickInfo);

%include "ui/BalloonPopupButtonClickInfo.h"

#endif
