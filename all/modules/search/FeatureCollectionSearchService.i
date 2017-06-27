#ifndef _FEATURECOLLECTIONSEARCHSERVICE_I
#define _FEATURECOLLECTIONSEARCHSERVICE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module(directors="1") FeatureCollectionSearchService

#ifdef _CARTO_SEARCH_SUPPORT

!proxy_imports(carto::FeatureCollectionSearchService, search.SearchRequest, geometry.FeatureCollection, projections.Projection)

%{
#include "search/FeatureCollectionSearchService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "search/SearchRequest.i"
%import "geometry/FeatureCollection.i"
%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::FeatureCollectionSearchService, search.FeatureCollectionSearchService)

%attributestring(carto::FeatureCollectionSearchService, std::shared_ptr<carto::Projection>, Projection, getProjection)
%attributestring(carto::FeatureCollectionSearchService, std::shared_ptr<carto::FeatureCollection>, FeatureCollection, getFeatureCollection)
%std_exceptions(carto::FeatureCollectionSearchService::FeatureCollectionSearchService)
%std_exceptions(carto::FeatureCollectionSearchService::findFeatures)

%feature("director") carto::FeatureCollectionSearchService;

%include "search/FeatureCollectionSearchService.h"

#endif

#endif
