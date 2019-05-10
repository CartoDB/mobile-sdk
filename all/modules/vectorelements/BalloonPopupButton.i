#ifndef _BALLOONPOPUPBUTTON_I
#define _BALLOONPOPUPBUTTON_I

#pragma SWIG nowarn=401

%module BalloonPopupButton

!proxy_imports(carto::BalloonPopupButton, core.Variant, styles.BalloonPopupButtonStyle)

%{
#include "vectorelements/BalloonPopupButton.h"
#include "core/Variant.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/Variant.i"
%import "styles/BalloonPopupButtonStyle.i"

!polymorphic_shared_ptr(carto::BalloonPopupButton, vectorelements.BalloonPopupButton)

%attributestring(carto::BalloonPopupButton, std::string, Text, getText)
!attributestring_polymorphic(carto::BalloonPopupButton, styles.BalloonPopupButtonStyle, Style, getStyle)
%attributeval(carto::BalloonPopupButton, carto::Variant, Tag, getTag, setTag)
%std_exceptions(carto::BalloonPopupButton::BalloonPopupButton)

%include "vectorelements/BalloonPopupButton.h"

#endif
