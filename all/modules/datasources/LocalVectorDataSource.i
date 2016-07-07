#ifndef _LOCALVECTORDATASOURCE_I
#define _LOCALVECTORDATASOURCE_I

%module(directors="1") LocalVectorDataSource

!proxy_imports(carto::LocalVectorDataSource, datasources.VectorDataSource, datasources.components.VectorData, geometry.GeometrySimplifier, projections.Projection, core.MapBounds, renderers.components.CullState, vectorelements.VectorElement, vectorelements.VectorElementVector)

%{
#include "datasources/LocalVectorDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/VectorDataSource.i"
%import "vectorelements/VectorElement.i"
%import "geometry/GeometrySimplifier.i"

!polymorphic_shared_ptr(carto::LocalVectorDataSource, datasources.LocalVectorDataSource)

%feature("director") carto::LocalVectorDataSource;

%attributeval(carto::LocalVectorDataSource, carto::MapBounds, DataExtent, getDataExtent)
!attributestring_polymorphic(carto::LocalVectorDataSource, geometry.GeometrySimplifier, GeometrySimplifier, getGeometrySimplifier, setGeometrySimplifier)
%std_exceptions(carto::LocalVectorDataSource::LocalVectorDataSource)
%std_exceptions(carto::LocalVectorDataSource::setAll)
%std_exceptions(carto::LocalVectorDataSource::add)
%std_exceptions(carto::LocalVectorDataSource::addAll)
%std_exceptions(carto::LocalVectorDataSource::remove)
%std_exceptions(carto::LocalVectorDataSource::removeAll)

%include "datasources/LocalVectorDataSource.h"

#endif
