#ifndef _CARTOPACKAGEMANAGER_I
#define _CARTOPACKAGEMANAGER_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module CartoPackageManager

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

!proxy_imports(carto::CartoPackageManager, packagemanager.PackageManager, layers.CartoVectorTileLayer, utils.AssetPackage)
!java_imports(carto::CartoPackageManager, com.carto.layers.CartoBaseMapStyle)

%{
#include "components/Exceptions.h"
#include "packagemanager/CartoPackageManager.h"
#include "layers/CartoVectorTileLayer.h"
#include "utils/AssetPackage.h"
#include <memory>
%}

%include <stdint.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "packagemanager/PackageManager.i"
%import "layers/CartoVectorTileLayer.i"
%import "utils/AssetPackage.i"

!polymorphic_shared_ptr(carto::CartoPackageManager, packagemanager.CartoPackageManager)

%std_io_exceptions(carto::CartoPackageManager::CartoPackageManager)

%include "packagemanager/CartoPackageManager.h"

#endif

#endif
