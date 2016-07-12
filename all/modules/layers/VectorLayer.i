#ifndef _VECTORLAYER_I
#define _VECTORLAYER_I

%module VectorLayer

!proxy_imports(carto::VectorLayer, datasources.VectorDataSource, layers.Layer, layers.VectorElementEventListener)

%{
#include "layers/VectorLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/VectorElementEventListener.i"
%import "layers/Layer.i"
%import "datasources/VectorDataSource.i"

!polymorphic_shared_ptr(carto::VectorLayer, layers.VectorLayer)

!attributestring_polymorphic(carto::VectorLayer, datasources.VectorDataSource, DataSource, getDataSource)
!attributestring_polymorphic(carto::VectorLayer, layers.VectorElementEventListener, VectorElementEventListener, getVectorElementEventListener, setVectorElementEventListener)
%std_exceptions(carto::VectorLayer::VectorLayer)

%include "layers/VectorLayer.h"

#endif
