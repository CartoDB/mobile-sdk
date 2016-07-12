#ifndef _VECTORDATASOURCE_I
#define _VECTORDATASOURCE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") VectorDataSource

!proxy_imports(carto::VectorDataSource, datasources.components.VectorData, projections.Projection, renderers.components.CullState, graphics.ViewState)

%{
#include "datasources/VectorDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/components/VectorData.i"
%import "projections/Projection.i"
%import "renderers/components/CullState.i"
%import "graphics/ViewState.i"

!polymorphic_shared_ptr(carto::VectorDataSource, datasources.VectorDataSource)

%feature("director") carto::VectorDataSource;

!attributestring_polymorphic(carto::VectorDataSource, projections.Projection, Projection, getProjection)
%std_exceptions(carto::VectorDataSource::VectorDataSource)
%ignore carto::VectorDataSource::OnChangeListener;
%ignore carto::VectorDataSource::registerOnChangeListener;
%ignore carto::VectorDataSource::unregisterOnChangeListener;
%ignore carto::VectorDataSource::calculateGeometrySimplifierScale;

%feature("nodirector") carto::VectorDataSource::notifyElementAdded;
%feature("nodirector") carto::VectorDataSource::notifyElementChanged;
%feature("nodirector") carto::VectorDataSource::notifyElementRemoved;
%feature("nodirector") carto::VectorDataSource::notifyElementsAdded;
%feature("nodirector") carto::VectorDataSource::notifyElementsRemoved;
%feature("nodirector") carto::VectorDataSource::attachElement;
%feature("nodirector") carto::VectorDataSource::detachElement;

%include "datasources/VectorDataSource.h"

#endif
