#ifndef _SEARCHREQUEST_I
#define _SEARCHREQUEST_I

#pragma SWIG nowarn=325

%module SearchRequest

#ifdef _CARTO_SEARCH_SUPPORT

!proxy_imports(carto::SearchRequest, geometry.Geometry, projections.Projection)

%{
#include "search/SearchRequest.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/Geometry.i"
%import "projections/Projection.i"

!shared_ptr(carto::SearchRequest, search.SearchRequest)

%attributestring(carto::SearchRequest, std::shared_ptr<carto::Projection>, Projection, getProjection)
%attributestring(carto::SearchRequest, std::string, FilterExpression, getFilterExpression, setFilterExpression)
%attributestring(carto::SearchRequest, std::string, RegexFilter, getRegexFilter, setRegexFilter)
%attributestring(carto::SearchRequest, std::shared_ptr<carto::Geometry>, Geometry, getGeometry, setGeometry)
%attribute(carto::SearchRequest, float, SearchRadius, getSearchRadius, setSearchRadius)
%std_exceptions(carto::SearchRequest::SearchRequest)
!standard_equals(carto::SearchRequest);
!custom_tostring(carto::SearchRequest);

%include "search/SearchRequest.h"

#endif

#endif
