#ifndef _VECTORELEMENTCLICKINFO_I
#define _VECTORELEMENTCLICKINFO_I

%module VectorElementClickInfo

!proxy_imports(carto::VectorElementClickInfo, core.MapPos, vectorelements.VectorElement, layers.Layer, ui.ClickType)

%{
#include "ui/VectorElementClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "ui/ClickType.i"
%import "core/MapPos.i"
%import "layers/Layer.i"
%import "vectorelements/VectorElement.i"

!shared_ptr(carto::VectorElementClickInfo, ui.VectorElementClickInfo)

%attribute(carto::VectorElementClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attributeval(carto::VectorElementClickInfo, carto::MapPos, ClickPos, getClickPos)
%attributeval(carto::VectorElementClickInfo, carto::MapPos, ElementClickPos, getElementClickPos)
!attributestring_polymorphic(carto::VectorElementClickInfo, vectorelements.VectorElement, VectorElement, getVectorElement)
!attributestring_polymorphic(carto::VectorElementClickInfo, layers.Layer, Layer, getLayer)
!standard_equals(carto::VectorElementClickInfo);

%include "ui/VectorElementClickInfo.h"

#endif
