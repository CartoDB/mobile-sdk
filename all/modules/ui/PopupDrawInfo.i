#ifndef _POPUPDRAWINFO_I
#define _POPUPDRAWINFO_I

%module PopupDrawInfo

!proxy_imports(carto::PopupDrawInfo, core.ScreenPos, core.ScreenBounds, vectorelements.Popup)

%{
#include "ui/PopupDrawInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/ScreenPos.i"
%import "core/ScreenBounds.i"
%import "vectorelements/Popup.i"

!shared_ptr(carto::PopupDrawInfo, ui.PopupDrawInfo)

%attributeval(carto::PopupDrawInfo, carto::ScreenPos, AnchorPoint, getAnchorPoint)
%attributeval(carto::PopupDrawInfo, carto::ScreenBounds, ScreenBounds, getScreenBounds)
!attributestring_polymorphic(carto::PopupDrawInfo, vectorelements.Popup, Popup, getPopup)
%attribute(carto::PopupDrawInfo, float, DPToPX, getDPToPX)
!standard_equals(carto::PopupDrawInfo);

%include "ui/PopupDrawInfo.h"

#endif
