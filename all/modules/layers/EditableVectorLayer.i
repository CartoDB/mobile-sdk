#ifndef _EDITABLEVECTORLAYER_I
#define _EDITABLEVECTORLAYER_I

%module EditableVectorLayer

#ifdef _CARTO_EDITABLE_SUPPORT

!proxy_imports(carto::EditableVectorLayer, datasources.VectorDataSource, layers.VectorLayer, layers.VectorEditEventListener, vectorelements.VectorElement)

%{
#include "layers/EditableVectorLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/VectorDataSource.i"
%import "layers/VectorLayer.i"
%import "layers/VectorEditEventListener.i"

!polymorphic_shared_ptr(carto::EditableVectorLayer, layers.EditableVectorLayer)

%attributestring(carto::EditableVectorLayer, std::shared_ptr<carto::VectorElement>, SelectedVectorElement, getSelectedVectorElement, setSelectedVectorElement)
!attributestring_polymorphic(carto::EditableVectorLayer, layers.VectorEditEventListener, VectorEditEventListener, getVectorEditEventListener, setVectorEditEventListener)
%std_exceptions(carto::EditableVectorLayer::EditableVectorLayer)

%include "layers/EditableVectorLayer.h"

#endif

#endif
