#ifndef _LABELSTYLEBUILDER_I
#define _LABELSTYLEBUILDER_I

%module LabelStyleBuilder

!proxy_imports(carto::LabelStyleBuilder, styles.BillboardStyleBuilder, styles.LabelStyle)

%{
#include "styles/LabelStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/BillboardStyleBuilder.i"
%import "styles/LabelStyle.i"

!polymorphic_shared_ptr(carto::LabelStyleBuilder, styles.LabelStyleBuilder)

%attribute(carto::LabelStyleBuilder, carto::BillboardOrientation::BillboardOrientation, OrientationMode, getOrientationMode, setOrientationMode)
%attribute(carto::LabelStyleBuilder, carto::BillboardScaling::BillboardScaling, ScalingMode, getScalingMode, setScalingMode)
%attribute(carto::LabelStyleBuilder, bool, Flippable, isFlippable, setFlippable)
%attribute(carto::LabelStyleBuilder, float, AnchorPointX, getAnchorPointX, setAnchorPointX)
%attribute(carto::LabelStyleBuilder, float, AnchorPointY, getAnchorPointY, setAnchorPointY)
!objc_rename(setAnchorPointX) carto::LabelStyleBuilder::setAnchorPoint(float, float);

%include "styles/LabelStyleBuilder.h"

#endif
