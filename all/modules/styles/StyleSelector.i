#ifndef _STYLESELECTOR_I
#define _STYLESELECTOR_I

%module StyleSelector

#ifdef _CARTO_GDAL_SUPPORT

%{
#include "styles/StyleSelector.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

!shared_ptr(carto::StyleSelector, styles.StyleSelector)

%ignore carto::StyleSelector::StyleSelector;
%ignore carto::StyleSelector::getStyle;
!standard_equals(carto::StyleSelector);

%include "styles/StyleSelector.h"

#endif

#endif
