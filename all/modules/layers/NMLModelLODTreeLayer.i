#ifndef _NMLMODELLODTREELAYER_I
#define _NMLMODELLODTREELAYER_I

%module NMLModelLODTreeLayer

!proxy_imports(carto::NMLModelLODTreeLayer, datasources.NMLModelLODTreeDataSource, layers.Layer)

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
%attribute(carto::NMLModelLODTreeLayer, unsigned int, MaxMemorySize, getMaxMemorySize, setMaxMemorySize)
%attribute(carto::NMLModelLODTreeLayer, float, LODResolutionFactor, getLODResolutionFactor, setLODResolutionFactor)

%include "layers/NMLModelLODTreeLayer.h"

#endif
