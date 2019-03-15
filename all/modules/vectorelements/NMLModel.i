#ifndef _NMLMODEL_I
#define _NMLMODEL_I

%module NMLModel

!proxy_imports(carto::NMLModel, core.BinaryData, core.MapBounds, core.MapPos, core.MapVec, geometry.Geometry, styles.NMLModelStyle, vectorelements.VectorElement)

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
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::NMLModel, vectorelements.NMLModel)

%attribute(carto::NMLModel, float, RotationAngle, getRotationAngle)
%attribute(carto::NMLModel, float, Scale, getScale, setScale)
%attributeval(carto::NMLModel, carto::MapVec, RotationAxis, getRotationAxis)
%attributestring(carto::NMLModel, std::shared_ptr<carto::NMLModelStyle>, Style, getStyle, setStyle)
%std_exceptions(carto::NMLModel::NMLModel)
%std_exceptions(carto::NMLModel::setGeometry)
%std_exceptions(carto::NMLModel::setStyle)
%ignore carto::NMLModel::getDrawData;
%ignore carto::NMLModel::setDrawData;

%include "vectorelements/NMLModel.h"

#endif
