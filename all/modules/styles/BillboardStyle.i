#ifndef _BILLBOARDSTYLE_I
#define _BILLBOARDSTYLE_I

%module BillboardStyle

!proxy_imports(carto::BillboardStyle, graphics.Color, styles.Style)

%{
#include "styles/BillboardStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/Style.i"

!polymorphic_shared_ptr(carto::BillboardStyle, styles.BillboardStyle)

%attribute(carto::BillboardStyle, bool, ScaleWithDPI, isScaleWithDPI)
%attribute(carto::BillboardStyle, int, PlacementPriority, getPlacementPriority)
%attribute(carto::BillboardStyle, bool, CausesOverlap, isCausesOverlap)
%attribute(carto::BillboardStyle, bool, HideIfOverlapped, isHideIfOverlapped)
%attribute(carto::BillboardStyle, float, AttachAnchorPointX, getAttachAnchorPointX)
%attribute(carto::BillboardStyle, float, AttachAnchorPointY, getAttachAnchorPointY)
%attribute(carto::BillboardStyle, float, HorizontalOffset, getHorizontalOffset)
%attribute(carto::BillboardStyle, float, VerticalOffset, getVerticalOffset)

%include "styles/BillboardStyle.h"

#endif
