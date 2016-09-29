#ifndef _OFFLINENMLMODELLODTREEDATASOURCE_I
#define _OFFLINENMLMODELLODTREEDATASOURCE_I

%module OfflineNMLModelLODTreeDataSource

#if defined(_CARTO_NMLMODELLODTREE_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

!proxy_imports(carto::OfflineNMLModelLODTreeDataSource, datasources.NMLModelLODTreeDataSource, projections.Projection)

%{
#include "datasources/OfflineNMLModelLODTreeDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/NMLModelLODTreeDataSource.i"

!polymorphic_shared_ptr(carto::OfflineNMLModelLODTreeDataSource, datasources.OfflineNMLModelLODTreeDataSource)

%std_io_exceptions(carto::OfflineNMLModelLODTreeDataSource::OfflineNMLModelLODTreeDataSource)

%include "datasources/OfflineNMLModelLODTreeDataSource.h"

#endif

#endif
