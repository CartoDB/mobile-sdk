#ifndef _VECTORDATASOURCESEARCHSERVICE_I
#define _VECTORDATASOURCESEARCHSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") VectorDataSourceSearchService

#ifdef _CARTO_SEARCH_SUPPORT

!proxy_imports(carto::VectorDataSourceSearchService, search.SearchRequest, datasources.VectorDataSource, vectorelements.VectorElement, projections.Projection)

%{
#include "search/VectorDataSourceSearchService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "search/SearchRequest.i"
%import "datasources/VectorDataSource.i"
%import "vectorelements/VectorElement.i"
%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::VectorDataSourceSearchService, search.VectorDataSourceSearchService)

%attributestring(carto::VectorDataSourceSearchService, std::shared_ptr<carto::VectorDataSource>, DataSource, getDataSource)
%std_exceptions(carto::VectorDataSourceSearchService::VectorDataSourceSearchService)
%std_exceptions(carto::VectorDataSourceSearchService::findElements)

%feature("director") carto::VectorDataSourceSearchService;

%include "search/VectorDataSourceSearchService.h"

#endif

#endif
