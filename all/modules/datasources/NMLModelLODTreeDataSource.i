#ifndef _NMLMODELLODTREEDATASOURCE_I
#define _NMLMODELLODTREEDATASOURCE_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module NMLModelLODTreeDataSource

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

!proxy_imports(carto::NMLModelLODTreeDataSource, projections.Projection)

%{
#include "datasources/NMLModelLODTreeDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "projections/Projection.i"

!polymorphic_shared_ptr(carto::NMLModelLODTreeDataSource, datasources.NMLModelLODTreeDataSource)

!attributestring_polymorphic(carto::NMLModelLODTreeDataSource, projections.Projection, Projection, getProjection);
%ignore carto::NMLModelLODTreeDataSource::MapTile;
%ignore carto::NMLModelLODTreeDataSource::loadMapTiles;
%ignore carto::NMLModelLODTreeDataSource::loadModelLODTree;
%ignore carto::NMLModelLODTreeDataSource::loadMesh;
%ignore carto::NMLModelLODTreeDataSource::loadTexture;

%include "datasources/NMLModelLODTreeDataSource.h"

#endif

#endif
