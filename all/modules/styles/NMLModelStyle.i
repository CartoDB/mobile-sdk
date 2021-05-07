#ifndef _NMLMODELSTYLE_I
#define _NMLMODELSTYLE_I

%module NMLModelStyle

!proxy_imports(carto::NMLModelStyle, core.BinaryData, graphics.Color, styles.BillboardStyle)

%{
#include "styles/NMLModelStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "styles/BillboardStyle.i"

!polymorphic_shared_ptr(carto::NMLModelStyle, styles.NMLModelStyle)

%attribute(carto::NMLModelStyle, carto::BillboardOrientation::BillboardOrientation, OrientationMode, getOrientationMode)
%attribute(carto::NMLModelStyle, carto::BillboardScaling::BillboardScaling, ScalingMode, getScalingMode)
%attributestring(carto::NMLModelStyle, std::shared_ptr<carto::BinaryData>, ModelAsset, getModelAsset)
%ignore carto::NMLModelStyle::getSourceModel;
%ignore carto::NMLModelStyle::NMLModelStyle;

%include "styles/NMLModelStyle.h"

#endif
