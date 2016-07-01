#ifndef _VECTORDATA_I
#define _VECTORDATA_I

%module VectorData

!proxy_imports(carto::VectorData, vectorelements.VectorElements, vectorelements.VectorElementVector)

%{
#include "datasources/components/VectorData.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "vectorelements/VectorElement.i"

!shared_ptr(carto::VectorData, datasources.components.VectorData)

%attributeval(carto::VectorData, %arg(std::vector<std::shared_ptr<carto::VectorElement> >), Elements, getElements)
!standard_equals(carto::VectorData);

%include "datasources/components/VectorData.h"

#endif
