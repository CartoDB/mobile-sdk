#ifndef _LICENSEMANAGER_I
#define _LICENSEMANAGER_I

%module LicenseManager

!proxy_imports(carto::LicenseManager, components.LicenseManagerListener)

%{
#include "components/LicenseManager.h"
#include "components/LicenseManagerListener.h"
%}

%include <std_string.i>
%include <cartoswig.i>

%import "components/LicenseManagerListener.i"

%attributestring(carto::LicenseManager, std::string, UserKey, getUserKey)
%ignore carto::LicenseManager::WatermarkType;
%ignore carto::LicenseManager::registerLicense;
%ignore carto::LicenseManager::getWatermarkType;
%ignore carto::LicenseManager::GetInstance;

%include "components/LicenseManager.h"

#endif
