#ifndef _POPUPCLICKINFO_I
#define _POPUPCLICKINFO_I

%module PopupClickInfo

!proxy_imports(carto::PopupClickInfo, core.MapPos, core.ScreenPos, vectorelements.Popup, ui.ClickInfo)

%{
#include "ui/PopupClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/ClickInfo.i"
%import "core/MapPos.i"
%import "core/ScreenPos.i"
%import "vectorelements/Popup.i"

!shared_ptr(carto::PopupClickInfo, ui.PopupClickInfo)

%attribute(carto::PopupClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attributeval(carto::PopupClickInfo, carto::ClickInfo, ClickInfo, getClickInfo)
%attributeval(carto::PopupClickInfo, carto::MapPos, ClickPos, getClickPos)
%attributeval(carto::PopupClickInfo, carto::ScreenPos, ElementClickPos, getElementClickPos)
!attributestring_polymorphic(carto::PopupClickInfo, vectorelements.Popup, Popup, getPopup)
%ignore carto::PopupClickInfo::PopupClickInfo;
!standard_equals(carto::PopupClickInfo);

%include "ui/PopupClickInfo.h"

#endif
