#ifndef _OGRVECTORDATASOURCE_I
#define _OGRVECTORDATASOURCE_I

%module(directors="1") OGRVectorDataSource

#ifdef _CARTO_GDAL_SUPPORT

!proxy_imports(carto::OGRVectorDataSource, core.MapBounds, core.StringVector, datasources.VectorDataSource, datasources.components.VectorData, datasources.OGRVectorDataBase, datasources.OGRFieldType, datasources.OGRGeometryType, geometry.GeometrySimplifier, projections.Projection, renderers.components.CullState, styles.StyleSelector)

%{
#include "datasources/OGRVectorDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/MapBounds.i"
%import "core/StringVector.i"
%import "datasources/VectorDataSource.i"
%import "datasources/OGRVectorDataBase.i"
%import "geometry/GeometrySimplifier.i"
%import "styles/StyleSelector.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::OGRVectorDataSource, datasources.OGRVectorDataSource)

%feature("director") carto::OGRVectorDataSource;

%attributeval(carto::OGRVectorDataSource, carto::MapBounds, DataExtent, getDataExtent)
%attribute(carto::OGRVectorDataSource, int, FeatureCount, getFeatureCount)
%attribute(carto::OGRVectorDataSource, OGRGeometryType::OGRGeometryType, GeometryType, getGeometryType)
%attributeval(carto::OGRVectorDataSource, std::vector<std::string>, FieldNames, getFieldNames)
%attributestring(carto::OGRVectorDataSource, std::string, CodePage, getCodePage, setCodePage)
!attributestring_polymorphic(carto::OGRVectorDataSource, geometry.GeometrySimplifier, GeometrySimplifier, getGeometrySimplifier, setGeometrySimplifier)
%std_exceptions(carto::OGRVectorDataSource::OGRVectorDataSource)
%std_exceptions(carto::OGRVectorDataSource::add)
%std_exceptions(carto::OGRVectorDataSource::remove)

%include "datasources/OGRVectorDataSource.h"

#endif

#endif
