#ifndef _MARKERSTYLEBUILDER_I
#define _MARKERSTYLEBUILDER_I

%module MarkerStyleBuilder

!proxy_imports(carto::MarkerStyleBuilder, graphics.Bitmap, styles.BillboardStyleBuilder, styles.MarkerStyle)

%{
#include "styles/MarkerStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/BillboardStyleBuilder.i"
%import "styles/MarkerStyle.i"

!polymorphic_shared_ptr(carto::MarkerStyleBuilder, styles.MarkerStyleBuilder)

%attribute(carto::MarkerStyleBuilder, float, Size, getSize, setSize)
%attribute(carto::MarkerStyleBuilder, carto::BillboardOrientation::BillboardOrientation, OrientationMode, getOrientationMode, setOrientationMode)
%attribute(carto::MarkerStyleBuilder, carto::BillboardScaling::BillboardScaling, ScalingMode, getScalingMode, setScalingMode)
%attribute(carto::MarkerStyleBuilder, float, AnchorPointX, getAnchorPointX, setAnchorPointX)
%attribute(carto::MarkerStyleBuilder, float, AnchorPointY, getAnchorPointY, setAnchorPointY)
%attributestring(carto::MarkerStyleBuilder, std::shared_ptr<carto::Bitmap>, Bitmap, getBitmap, setBitmap)
%std_exceptions(carto::MarkerStyleBuilder::setBitmap)
!objc_rename(setAnchorPointX) carto::MarkerStyleBuilder::setAnchorPoint(float, float);

%include "styles/MarkerStyleBuilder.h"

#endif
