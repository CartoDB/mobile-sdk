#ifndef _ONLINENMLMODELLODTREEDATASOURCE_I
#define _ONLINENMLMODELLODTREEDATASOURCE_I

%module OnlineNMLModelLODTreeDataSource

!proxy_imports(carto::OnlineNMLModelLODTreeDataSource, datasources.NMLModelLODTreeDataSource, projections.Projection)

%{
#include "datasources/OnlineNMLModelLODTreeDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/NMLModelLODTreeDataSource.i"

!polymorphic_shared_ptr(carto::OnlineNMLModelLODTreeDataSource, datasources.OnlineNMLModelLODTreeDataSource)

%include "datasources/OnlineNMLModelLODTreeDataSource.h"

#endif
