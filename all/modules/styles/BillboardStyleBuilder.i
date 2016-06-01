#ifndef _BILLBOARDSTYLEBUILDER_I
#define _BILLBOARDSTYLEBUILDER_I

%module BillboardStyleBuilder

!proxy_imports(carto::BillboardStyleBuilder, styles.StyleBuilder)

%{
#include "styles/BillboardStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/BillboardStyle.i"
%import "styles/StyleBuilder.i"

!polymorphic_shared_ptr(carto::BillboardStyleBuilder, styles.BillboardStyleBuilder)

%attribute(carto::BillboardStyleBuilder, bool, ScaleWithDPI, isScaleWithDPI, setScaleWithDPI)
%attribute(carto::BillboardStyleBuilder, int, PlacementPriority, getPlacementPriority, setPlacementPriority)
%attribute(carto::BillboardStyleBuilder, bool, CausesOverlap, isCausesOverlap, setCausesOverlap)
%attribute(carto::BillboardStyleBuilder, bool, HideIfOverlapped, isHideIfOverlapped, setHideIfOverlapped)
%attribute(carto::BillboardStyleBuilder, float, AttachAnchorPointX, getAttachAnchorPointX, setAttachAnchorPointX)
%attribute(carto::BillboardStyleBuilder, float, AttachAnchorPointY, getAttachAnchorPointY, setAttachAnchorPointY)
%attribute(carto::BillboardStyleBuilder, float, HorizontalOffset, getHorizontalOffset, setHorizontalOffset)
%attribute(carto::BillboardStyleBuilder, float, VerticalOffset, getVerticalOffset, setVerticalOffset)
!objc_rename(setAttachAnchorPointX) carto::BillboardStyleBuilder::setAttachAnchorPoint(float, float);

%include "styles/BillboardStyleBuilder.h"

#endif
