#ifndef _CUSTOMPOPUP_I
#define _CUSTOMPOPUP_I

%module CustomPopup

!proxy_imports(carto::CustomPopup, core.MapPos, core.ScreenPos, graphics.Bitmap, geometry.Geometry, geometry.PointGeometry, styles.PopupStyle, ui.ClickType, ui.PopupDrawInfo, ui.PopupClickInfo, vectorelements.Billboard, vectorelements.CustomPopupHandler)

%{
#include "vectorelements/CustomPopup.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/ScreenPos.i"
%import "graphics/Bitmap.i"
%import "styles/PopupStyle.i"
%import "ui/PopupDrawInfo.i"
%import "ui/PopupClickInfo.i"
%import "vectorelements/Billboard.i"
%import "vectorelements/CustomPopupHandler.i"

!polymorphic_shared_ptr(carto::CustomPopup, vectorelements.CustomPopup)

!attributestring_polymorphic(carto::CustomPopup, vectorelements.CustomPopupHandler, PopupHandler, getPopupHandler)

%include "vectorelements/CustomPopup.h"

#endif
