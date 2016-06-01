#ifndef _CLUSTERELEMENTBUILDER_I
#define _CLUSTERELEMENTBUILDER_I

%module(directors="1") ClusterElementBuilder
!proxy_imports(carto::ClusterElementBuilder, core.MapPos, vectorelements.VectorElement, vectorelements.VectorElementVector)

%{
#include "layers/ClusterElementBuilder.h"
#include "core/MapPos.h"
#include "vectorelements/VectorElement.h"
#include <memory>
%}

%include <std_shared_ptr.i>

%import "core/MapPos.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::ClusterElementBuilder, layers.ClusterElementBuilder)

%feature("director") carto::ClusterElementBuilder;

%include "layers/ClusterElementBuilder.h"

#endif
