#ifndef _FEATURECOLLECTION_I
#define _FEATURECOLLECTION_I

%module FeatureCollection

!proxy_imports(carto::FeatureCollection, geometry.Feature, geometry.FeatureVector)

%{
#include "geometry/FeatureCollection.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/Feature.i"

!shared_ptr(carto::FeatureCollection, geometry.FeatureCollection)

%attribute(carto::FeatureCollection, int, FeatureCount, getFeatureCount)
%std_exceptions(carto::FeatureCollection::getFeature)

%include "geometry/FeatureCollection.h"

#endif
