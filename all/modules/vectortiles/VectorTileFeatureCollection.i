#ifndef _VECTORTILEFEATURECOLLECTION_I
#define _VECTORTILEFEATURECOLLECTION_I

%module VectorTileFeatureCollection

!proxy_imports(carto::VectorTileFeatureCollection, vectortiles.VectorTileFeature, vectortiles.VectorTileFeatureVector)

%{
#include "vectortiles/VectorTileFeatureCollection.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "vectortiles/VectorTileFeature.i"

!shared_ptr(carto::VectorTileFeatureCollection, vectortiles.VectorTileFeatureCollection)

%attribute(carto::VectorTileFeatureCollection, int, FeatureCount, getFeatureCount)
%std_exceptions(carto::VectorTileFeatureCollection::getFeature)

%include "vectortiles/VectorTileFeatureCollection.h"

#endif
