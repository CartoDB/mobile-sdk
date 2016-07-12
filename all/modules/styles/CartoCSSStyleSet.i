#ifndef _CARTOCSSSTYLESET_I
#define _CARTOCSSSTYLESET_I

%module CartoCSSStyleSet

!proxy_imports(carto::CartoCSSStyleSet, utils.AssetPackage)

%{
#include "styles/CartoCSSStyleSet.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "utils/AssetPackage.i"

!shared_ptr(carto::CartoCSSStyleSet, styles.CartoCSSStyleSet)

%attributestring(carto::CartoCSSStyleSet, std::string, CartoCSS, getCartoCSS)
%attributestring(carto::CartoCSSStyleSet, std::shared_ptr<carto::AssetPackage>, AssetPackage, getAssetPackage)
!standard_equals(carto::CartoCSSStyleSet);

%include "styles/CartoCSSStyleSet.h"

#endif
