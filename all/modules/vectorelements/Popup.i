#ifndef _POPUP_I
#define _POPUP_I

%module(directors="1") Popup

!proxy_imports(carto::Popup, core.MapPos, core.ScreenPos, graphics.Bitmap, geometry.Geometry, geometry.PointGeometry, styles.PopupStyle, vectorelements.Billboard)

%{
#include "vectorelements/Popup.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/ScreenPos.i"
%import "graphics/Bitmap.i"
%import "styles/PopupStyle.i"
%import "vectorelements/Billboard.i"

!polymorphic_shared_ptr(carto::Popup, vectorelements.Popup)

%feature("director") carto::Popup;

%attribute(carto::Popup, float, AnchorPointX, getAnchorPointX)
%attribute(carto::Popup, float, AnchorPointY, getAnchorPointY)
!attributestring_polymorphic(carto::Popup, styles.PopupStyle, Style, getStyle, setStyle)
!objc_rename(setAnchorPointX) carto::Popup::setAnchorPoint;

%feature("nodirector") carto::Popup::getBounds;

%include "vectorelements/Popup.h"

#endif
