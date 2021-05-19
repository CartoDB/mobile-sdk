#ifndef _NMLMODEL_I
#define _NMLMODEL_I

%module NMLModel

!proxy_imports(carto::NMLModel, core.BinaryData, core.MapBounds, core.MapPos, core.MapVec, geometry.Geometry, styles.NMLModelStyle, vectorelements.Billboard)

%{
#include "vectorelements/NMLModel.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "core/MapPos.i"
%import "geometry/Geometry.i"
%import "styles/NMLModelStyle.i"
%import "vectorelements/Billboard.i"

!polymorphic_shared_ptr(carto::NMLModel, vectorelements.NMLModel)

%attribute(carto::NMLModel, float, RotationAngle, getRotationAngle, setRotationAngle)
%attributeval(carto::NMLModel, carto::MapVec, RotationAxis, getRotationAxis, setRotationAxis)
%attributestring(carto::NMLModel, std::shared_ptr<carto::NMLModelStyle>, Style, getStyle, setStyle)
%attribute(carto::NMLModel, float, Scale, getScale, setScale)
%std_exceptions(carto::NMLModel::NMLModel)
%std_exceptions(carto::NMLModel::setGeometry)
%std_exceptions(carto::NMLModel::setStyle)

%include "vectorelements/NMLModel.h"

#endif
