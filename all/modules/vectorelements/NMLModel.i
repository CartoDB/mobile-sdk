#ifndef _NMLMODEL_I
#define _NMLMODEL_I

%module NMLModel

!proxy_imports(carto::NMLModel, core.BinaryData, core.MapBounds, core.MapPos, core.MapVec, geometry.Geometry, vectorelements.VectorElement)

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
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::NMLModel, vectorelements.NMLModel)

%attribute(carto::NMLModel, float, RotationAngle, getRotationAngle)
%attribute(carto::NMLModel, float, Scale, getScale, setScale)
%attributeval(carto::NMLModel, carto::MapVec, RotationAxis, getRotationAxis)
%std_exceptions(carto::NMLModel::NMLModel)
%std_exceptions(carto::NMLModel::setGeometry)
%csmethodmodifiers carto::NMLModel::Bounds "public override";
%ignore carto::NMLModel::NMLModel(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<nml::Model>& sourceModel);
%ignore carto::NMLModel::NMLModel(const MapPos& pos, const std::shared_ptr<nml::Model>& sourceModel);
%ignore carto::NMLModel::getLocalMat;
%ignore carto::NMLModel::getSourceModel;
%ignore carto::NMLModel::getDrawData;
%ignore carto::NMLModel::setDrawData;

%include "vectorelements/NMLModel.h"

#endif
