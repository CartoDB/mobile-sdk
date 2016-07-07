#ifndef _NMLMODELLODTREELAYER_I
#define _NMLMODELLODTREELAYER_I

%module NMLModelLODTreeLayer

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

!proxy_imports(carto::NMLModelLODTreeLayer, datasources.NMLModelLODTreeDataSource, layers.Layer, layers.NMLModelLODTreeEventListener)

%{
#include "layers/NMLModelLODTreeLayer.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/NMLModelLODTreeDataSource.i"
%import "layers/Layer.i"

!polymorphic_shared_ptr(carto::NMLModelLODTreeLayer, layers.NMLModelLODTreeLayer)

!attributestring_polymorphic(carto::NMLModelLODTreeLayer, datasources.NMLModelLODTreeDataSource, DataSource, getDataSource);
%attribute(carto::NMLModelLODTreeLayer, std::size_t, MaxMemorySize, getMaxMemorySize, setMaxMemorySize)
%attribute(carto::NMLModelLODTreeLayer, float, LODResolutionFactor, getLODResolutionFactor, setLODResolutionFactor)
%std_exceptions(carto::NMLModelLODTreeLayer::NMLModelLODTreeLayer)

%include "layers/NMLModelLODTreeLayer.h"

#endif

#endif
