#ifndef _LOCALVECTORDATASOURCE_I
#define _LOCALVECTORDATASOURCE_I

%module(directors="1") LocalVectorDataSource

!proxy_imports(carto::LocalVectorDataSource, datasources.VectorDataSource, geometry.GeometrySimplifier, projections.Projection, core.MapBounds, renderers.components.CullState, vectorelements.VectorElement, vectorelements.VectorElementVector)

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

%include "datasources/LocalVectorDataSource.h"

#endif
