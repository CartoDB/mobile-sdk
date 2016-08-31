#ifndef _VECTORELEMENTDRAGINFO_I
#define _VECTORELEMENTDRAGINFO_I

%module VectorElementDragInfo

#ifdef _CARTO_EDITABLE_SUPPORT

!proxy_imports(carto::VectorElementDragInfo, core.MapPos, core.ScreenPos, vectorelements.VectorElement)

%{
#include "ui/VectorElementDragInfo.h"
#include <memory>
%}

%import <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/ScreenPos.i"
%import "vectorelements/VectorElement.i"

!shared_ptr(carto::VectorElementDragInfo, ui.VectorElementDragInfo)

%attributeval(carto::VectorElementDragInfo, carto::ScreenPos, ScreenPos, getScreenPos)
%attributeval(carto::VectorElementDragInfo, carto::MapPos, MapPos, getMapPos)
%attribute(carto::VectorElementDragInfo, carto::VectorElementDragMode::VectorElementDragMode, VectorElementDragMode, getDragMode)
!attributestring_polymorphic(carto::VectorElementDragInfo, vectorelements.VectorElement, VectorElement, getVectorElement)
%ignore carto::VectorElementDragInfo::VectorElementDragInfo;
!standard_equals(carto::VectorElementDragInfo);

%include "ui/VectorElementDragInfo.h"

#endif

#endif
