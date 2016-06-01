#ifndef _NMLMODELLODTREE_I
#define _NMLMODELLODTREE_I

#pragma SWIG nowarn=325

%module NMLModelLODTreeProxy

!proxy_imports(carto::NMLModelLODTreeProxy, core.MapPos, geometry.PointGeometry, vectorelements.VectorElement)

%{
#include "geometry/PointGeometry.h"
#include "vectorelements/NMLModelLODTree.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/PointGeometry.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::NMLModelLODTree, vectorelements.NMLModelLODTree)

!polymorphic_shared_ptr(carto::NMLModelLODTreeProxy, vectorelements.NMLModelLODTreeProxy)

%ignore carto::NMLModelLODTree;

%csmethodmodifiers carto::NMLModelLODTreeProxy::Geometry "public new";
!attributestring_polymorphic(carto::NMLModelLODTreeProxy, geometry.PointGeometry, Geometry, getGeometry)

%include "vectorelements/NMLModelLODTree.h"

#endif
