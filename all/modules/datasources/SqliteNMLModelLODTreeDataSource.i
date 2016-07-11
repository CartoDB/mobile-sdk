#ifndef _SQLITENMLMODELLODTREEDATASOURCE_I
#define _SQLITENMLMODELLODTREEDATASOURCE_I

%module SqliteNMLModelLODTreeDataSource

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

!proxy_imports(carto::SqliteNMLModelLODTreeDataSource, datasources.NMLModelLODTreeDataSource, projections.Projection)

%{
#include "datasources/SqliteNMLModelLODTreeDataSource.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/NMLModelLODTreeDataSource.i"

!polymorphic_shared_ptr(carto::SqliteNMLModelLODTreeDataSource, datasources.SqliteNMLModelLODTreeDataSource)

%std_io_exceptions(carto::SqliteNMLModelLODTreeDataSource::SqliteNMLModelLODTreeDataSource)

%include "datasources/SqliteNMLModelLODTreeDataSource.h"

#endif

#endif
