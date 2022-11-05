#ifndef _LOCALVECTORDATASOURCE_I
#define _LOCALVECTORDATASOURCE_I

%module(directors="1") LocalVectorDataSource

!proxy_imports(carto::LocalVectorDataSource, core.MapBounds, datasources.VectorDataSource, datasources.components.VectorData, geometry.FeatureCollection, geometry.GeometrySimplifier, projections.Projection, renderers.components.CullState, styles.Style, vectorelements.VectorElement, vectorelements.VectorElementVector)

%{
#include "datasources/LocalVectorDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/VectorDataSource.i"
%import "geometry/GeometrySimplifier.i"
%import "geometry/FeatureCollection.i"
%import "styles/Style.i"
%import "vectorelements/VectorElement.i"

!enum(carto::LocalSpatialIndexType::LocalSpatialIndexType)
!polymorphic_shared_ptr(carto::LocalVectorDataSource, datasources.LocalVectorDataSource)

%feature("director") carto::LocalVectorDataSource;

!attributestring_polymorphic(carto::LocalVectorDataSource, geometry.GeometrySimplifier, GeometrySimplifier, getGeometrySimplifier, setGeometrySimplifier)
%std_exceptions(carto::LocalVectorDataSource::LocalVectorDataSource)
%std_exceptions(carto::LocalVectorDataSource::setAll)
%std_exceptions(carto::LocalVectorDataSource::add)
%std_exceptions(carto::LocalVectorDataSource::addAll)
%std_exceptions(carto::LocalVectorDataSource::remove)
%std_exceptions(carto::LocalVectorDataSource::removeAll)
%std_exceptions(carto::LocalVectorDataSource::addFeatureCollection)

%include "datasources/LocalVectorDataSource.h"

#endif
