#ifndef _POPUPSTYLEBUILDER_I
#define _POPUPSTYLEBUILDER_I
        
%module PopupStyleBuilder

!proxy_imports(carto::PopupStyleBuilder, styles.BillboardStyleBuilder, styles.PopupStyle)

%{
#include "styles/PopupStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/BillboardStyleBuilder.i"
%import "styles/PopupStyle.i"

!polymorphic_shared_ptr(carto::PopupStyleBuilder, styles.PopupStyleBuilder)

%include "styles/PopupStyleBuilder.h"

#endif
