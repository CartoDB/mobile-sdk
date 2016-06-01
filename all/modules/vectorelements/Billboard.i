#ifndef _BILLBOARD_I
#define _BILLBOARD_I

%module Billboard

!proxy_imports(carto::Billboard, core.MapBounds, core.MapPos, geometry.Geometry, styles.BillboardStyle, vectorelements.VectorElement)

%{
#include "vectorelements/Billboard.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/Geometry.i"
%import "styles/BillboardStyle.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::Billboard, vectorelements.Billboard)

%attribute(carto::Billboard, float, Rotation, getRotation, setRotation)
%csmethodmodifiers carto::Billboard::Bounds "public override";
%attributeval(carto::Billboard, carto::MapBounds, Bounds, getBounds)
!attributestring_polymorphic(carto::Billboard, geometry.Geometry, RootGeometry, getRootGeometry)
%csmethodmodifiers carto::Billboard::Geometry "public new";
!attributestring_polymorphic(carto::Billboard, geometry.Geometry, Geometry, getGeometry, setGeometry)
%attributestring(carto::Billboard, std::shared_ptr<carto::Billboard>, BaseBillboard, getBaseBillboard, setBaseBillboard)
%ignore carto::Billboard::getDrawData;
%ignore carto::Billboard::setDrawData;

%include "vectorelements/Billboard.h"

#endif
