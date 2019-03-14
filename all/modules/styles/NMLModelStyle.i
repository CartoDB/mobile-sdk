#ifndef _NMLMODELSTYLE_I
#define _NMLMODELSTYLE_I

%module NMLModelStyle

!proxy_imports(carto::NMLModelStyle, core.BinaryData, graphics.Color, styles.Style)

%{
#include "styles/NMLModelStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "styles/Style.i"

!polymorphic_shared_ptr(carto::NMLModelStyle, styles.NMLModelStyle)

%ignore carto::NMLModelStyle::getSourceModel;
%ignore carto::NMLModelStyle::NMLModelStyle;

%include "styles/NMLModelStyle.h"

#endif
