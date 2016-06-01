#ifndef _CARTOPACKAGEMANAGER_I
#define _CARTOPACKAGEMANAGER_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module CartoPackageManager

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

!proxy_imports(carto::CartoPackageManager, packagemanager.PackageManager)

%{
#include "packagemanager/CartoPackageManager.h"
%}

%include <stdint.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "packagemanager/PackageManager.i"

!polymorphic_shared_ptr(carto::CartoPackageManager, packagemanager.CartoPackageManager)

%include "packagemanager/CartoPackageManager.h"

#endif

#endif
