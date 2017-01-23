#ifndef _GEOCODINGRESULT_I
#define _GEOCODINGRESULT_I

#pragma SWIG nowarn=325

%module GeocodingResult

#ifdef _CARTO_GEOCODING_SUPPORT

!proxy_imports(carto::GeocodingResult, geocoding.GeocodingAddress, geometry.FeatureCollection, projections.Projection)

%{
#include "geocoding/GeocodingResult.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "geocoding/GeocodingAddress.i"
%import "geometry/FeatureCollection.i"
%import "projections/Projection.i"

!shared_ptr(carto::GeocodingResult, geocoding.GeocodingResult)

%attributestring(carto::GeocodingResult, std::shared_ptr<carto::Projection>, Projection, getProjection)
%attributestring(carto::GeocodingResult, std::shared_ptr<carto::FeatureCollection>, FeatureCollection, getFeatureCollection)
%attributeval(carto::GeocodingResult, carto::GeocodingAddress, Address, getAddress)
%attribute(carto::GeocodingResult, float, Rank, getRank)
%std_exceptions(carto::GeocodingResult::GeocodingResult)
!standard_equals(carto::GeocodingResult);
!custom_tostring(carto::GeocodingResult);

%include "geocoding/GeocodingResult.h"

!value_template(std::vector<std::shared_ptr<carto::GeocodingResult> >, geocoding.GeocodingResultVector);

#endif

#endif
