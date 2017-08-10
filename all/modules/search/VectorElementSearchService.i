#ifndef _VECTORELEMENTSEARCHSERVICE_I
#define _VECTORELEMENTSEARCHSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") VectorElementSearchService

#ifdef _CARTO_SEARCH_SUPPORT

!proxy_imports(carto::VectorElementSearchService, search.SearchRequest, datasources.VectorDataSource, vectorelements.VectorElement, vectorelements.VectorElementVector, projections.Projection)

%{
#include "search/VectorElementSearchService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "search/SearchRequest.i"
%import "datasources/VectorDataSource.i"
%import "vectorelements/VectorElement.i"
%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::VectorElementSearchService, search.VectorElementSearchService)

%attributestring(carto::VectorElementSearchService, std::shared_ptr<carto::VectorDataSource>, DataSource, getDataSource)
%std_exceptions(carto::VectorElementSearchService::VectorElementSearchService)
%std_exceptions(carto::VectorElementSearchService::findElements)

%feature("director") carto::VectorElementSearchService;

%include "search/VectorElementSearchService.h"

#endif

#endif
