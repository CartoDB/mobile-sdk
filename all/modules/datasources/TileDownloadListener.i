#ifndef _TILEDOWNLOADLISTENER_I
#define _TILEDOWNLOADLISTENER_I

%module(directors="1") TileDownloadListener

!proxy_imports(carto::TileDownloadListener, core.MapTile)

%{
#include "datasources/TileDownloadListener.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapTile.i"

!polymorphic_shared_ptr(carto::TileDownloadListener, datasources.TileDownloadListener)

%feature("director") carto::TileDownloadListener;

%include "datasources/TileDownloadListener.h"

#endif
