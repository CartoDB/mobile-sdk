#ifndef _VECTORTILEFEATURECOLLECTION_I
#define _VECTORTILEFEATURECOLLECTION_I

%module VectorTileFeatureCollection

!proxy_imports(carto::VectorTileFeatureCollection, geometry.FeatureCollection, geometry.VectorTileFeature, geometry.VectorTileFeatureVector)

%{
#include "geometry/VectorTileFeatureCollection.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/FeatureCollection.i"
%import "geometry/VectorTileFeature.i"

!polymorphic_shared_ptr(carto::VectorTileFeatureCollection, geometry.VectorTileFeatureCollection)

%std_exceptions(carto::VectorTileFeatureCollection::getFeature)

%include "geometry/VectorTileFeatureCollection.h"

#endif
