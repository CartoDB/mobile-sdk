#ifndef _LABELSTYLE_I
#define _LABELSTYLE_I

%module LabelStyle

!proxy_imports(carto::LabelStyle, graphics.Color, styles.BillboardStyle)

%{
#include "styles/LabelStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"
%import "styles/BillboardStyle.i"

!polymorphic_shared_ptr(carto::LabelStyle, styles.LabelStyle)

%attribute(carto::LabelStyle, carto::BillboardOrientation::BillboardOrientation, OrientationMode, getOrientationMode)
%attribute(carto::LabelStyle, carto::BillboardScaling::BillboardScaling, ScalingMode, getScalingMode)
%attribute(carto::LabelStyle, bool, Flippable, isFlippable)
%attribute(carto::LabelStyle, float, AnchorPointX, getAnchorPointX)
%attribute(carto::LabelStyle, float, AnchorPointY, getAnchorPointY)

%include "styles/LabelStyle.h"

#endif
