#ifndef _MARKERSTYLE_I
#define _MARKERSTYLE_I

%module MarkerStyle

!proxy_imports(carto::MarkerStyle, graphics.Bitmap, graphics.Color, styles.BillboardStyle)

%{
#include "styles/MarkerStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"
%import "styles/BillboardStyle.i"

!polymorphic_shared_ptr(carto::MarkerStyle, styles.MarkerStyle)

%attribute(carto::MarkerStyle, float, Size, getSize)
%attribute(carto::MarkerStyle, carto::BillboardOrientation::BillboardOrientation, OrientationMode, getOrientationMode)
%attribute(carto::MarkerStyle, carto::BillboardScaling::BillboardScaling, ScalingMode, getScalingMode)
%attribute(carto::MarkerStyle, float, AnchorPointX, getAnchorPointX)
%attribute(carto::MarkerStyle, float, AnchorPointY, getAnchorPointY)
%attributestring(carto::MarkerStyle, std::shared_ptr<carto::Bitmap>, Bitmap, getBitmap)

%include "styles/MarkerStyle.h"

#endif
