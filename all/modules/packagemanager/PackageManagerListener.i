#ifndef _PACKAGEMANAGERLISTENER_I
#define _PACKAGEMANAGERLISTENER_I

%module(directors="1") PackageManagerListener

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

!proxy_imports(carto::PackageManagerListener, packagemanager.PackageStatus)

%{
#include "packagemanager/PackageManagerListener.h"	
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>

%import "packagemanager/PackageStatus.i"

!polymorphic_shared_ptr(carto::PackageManagerListener, packagemanager.PackageManagerListener)

%feature("director") carto::PackageManagerListener;

%include "packagemanager/PackageManagerListener.h"

#endif

#endif
