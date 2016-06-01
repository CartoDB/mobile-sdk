#ifndef _CARTOCSSSTYLESET_I
#define _CARTOCSSSTYLESET_I

%module CartoCSSStyleSet

!proxy_imports(carto::CartoCSSStyleSet, vectortiles.AssetPackage)

%{
#include "vectortiles/AssetPackage.h"
#include "vectortiles/CartoCSSStyleSet.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "vectortiles/AssetPackage.i"

!shared_ptr(carto::CartoCSSStyleSet, vectortiles.CartoCSSStyleSet)

%attributestring(carto::CartoCSSStyleSet, std::string, CartoCSS, getCartoCSS)
%attributestring(carto::CartoCSSStyleSet, std::shared_ptr<carto::AssetPackage>, AssetPackage, getAssetPackage)
!standard_equals(carto::CartoCSSStyleSet);

%include "vectortiles/CartoCSSStyleSet.h"

#endif
