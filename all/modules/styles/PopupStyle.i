#ifndef _POPUPSTYLE_I
#define _POPUPSTYLE_I

%module PopupStyle

!proxy_imports(carto::PopupStyle, graphics.Color, styles.BillboardStyle)

%{
#include "styles/PopupStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"
%import "styles/BillboardStyle.i"

!polymorphic_shared_ptr(carto::PopupStyle, styles.PopupStyle)

%include "styles/PopupStyle.h"

#endif
