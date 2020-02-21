#ifndef _ROUTEMATCHINGEDGE_I
#define _ROUTEMATCHINGEDGE_I

%module RouteMatchingEdge

#ifdef _CARTO_ROUTING_SUPPORT

!proxy_imports(carto::RouteMatchingEdge, core.Variant, core.StringVariantMap)

%{
#include "routing/RouteMatchingEdge.h"
%}

%include <std_string.i>
%include <cartoswig.i>

%import "core/Variant.i"

!value_type(carto::RouteMatchingEdge, routing.RouteMatchingEdge)
!value_type(std::vector<carto::RouteMatchingEdge>, routing.RouteMatchingEdgeVector)

!standard_equals(carto::RouteMatchingEdge);
!custom_tostring(carto::RouteMatchingEdge);

%include "routing/RouteMatchingEdge.h"

!value_template(std::vector<carto::RouteMatchingEdge>, routing.RouteMatchingEdgeVector);

#endif

#endif
