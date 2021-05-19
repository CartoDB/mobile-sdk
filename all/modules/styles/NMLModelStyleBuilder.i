#ifndef _NMLMODELSTYLEBUILDER_I
#define _NMLMODELSTYLEBUILDER_I

%module NMLModelStyleBuilder

!proxy_imports(carto::NMLModelStyleBuilder, core.BinaryData, styles.NMLModelStyle, styles.BillboardStyleBuilder)

%{
#include "styles/NMLModelStyleBuilder.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "styles/NMLModelStyle.i"
%import "styles/BillboardStyleBuilder.i"

!polymorphic_shared_ptr(carto::NMLModelStyleBuilder, styles.NMLModelStyleBuilder)

%attribute(carto::NMLModelStyleBuilder, carto::BillboardOrientation::BillboardOrientation, OrientationMode, getOrientationMode, setOrientationMode)
%attribute(carto::NMLModelStyleBuilder, carto::BillboardScaling::BillboardScaling, ScalingMode, getScalingMode, setScalingMode)
%attributestring(carto::NMLModelStyleBuilder, std::shared_ptr<carto::BinaryData>, ModelAsset, getModelAsset, setModelAsset)
%std_exceptions(carto::NMLModelStyleBuilder::setModelAsset)

%include "styles/NMLModelStyleBuilder.h"

#endif
