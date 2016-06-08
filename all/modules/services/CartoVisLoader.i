#ifndef _CARTOVISLOADER_I
#define _CARTOVISLOADER_I

%module CartoVisLoader

!proxy_imports(carto::CartoVisLoader, services.CartoUIBuilder, ui.BaseMapView)

%{
#include "services/CartoVisLoader.h"
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "services/CartoUIBuilder.i"
%import "ui/BaseMapView.i"

!shared_ptr(carto::CartoVisLoader, services.CartoVisLoader)

!attributestring_polymorphic(carto::CartoVisLoader, services.CartoUIBuilder, CartoUIBuilder, getCartoUIBuilder, setCartoUIBuilder)
!standard_equals(carto::CartoVisLoader);

%include "services/CartoVisLoader.h"

#endif
