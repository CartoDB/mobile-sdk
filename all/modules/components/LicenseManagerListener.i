#ifndef _LICENSEMANAGERLISTENER_I
#define _LICENSEMANAGERLISTENER_I

%module(directors="1") LicenseManagerListener

%{
#include "components/LicenseManagerListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>

!polymorphic_shared_ptr(carto::LicenseManagerListener, components.LicenseManagerListener)

%feature("director") carto::LicenseManagerListener;

%include "components/LicenseManagerListener.h"

#endif
