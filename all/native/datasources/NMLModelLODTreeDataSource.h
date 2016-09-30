/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELLODTREEDATASOURCE_H_
#define _CARTO_NMLMODELLODTREEDATASOURCE_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "datasources/components/NMLModelLODTree.h"

#include <memory>
#include <mutex>

namespace carto {
    class CullState;
    class Projection;
    
    /**
     * An abstract base class for NML model LOD tree data sources. NML model LOD tree data sources are tile based data sources
     * that provide access to different entities: map tiles, LOD tree metadata, model meshes and textures.
     */
    class NMLModelLODTreeDataSource : public std::enable_shared_from_this<NMLModelLODTreeDataSource> {
    public:
        /**
         * Map tile containing reference to LOD tree.
         */
        struct MapTile {
            long long mapTileId;
            MapPos mapPos;
            long long modelLODTreeId;
    
            MapTile(long long mapTileId, const MapPos& mapPos, long long modelLODTreeId) : mapTileId(mapTileId), mapPos(mapPos), modelLODTreeId(modelLODTreeId) { }
        };
    
        virtual ~NMLModelLODTreeDataSource();
    
        /**
         * Returns the projection used by this data source.
         * @return The projection used by this data source.
         */
        std::shared_ptr<Projection> getProjection() const;
    
        /**
         * Loads map tiles based on given view state.
         * @param cullState The cull state for map tiles.
         * @return A list of map tiles visible from the given cull state.
         */
        virtual std::vector<MapTile> loadMapTiles(const std::shared_ptr<CullState>& cullState) = 0;
    
        /**
         * Loads model LOD tree description for the given tile.
         * @param mapTile The map tile to be loaded.
         * @return LOD tree for the map tile or null if LOD tree could not be loaded.
         */
        virtual std::shared_ptr<NMLModelLODTree> loadModelLODTree(const MapTile& mapTile) = 0;
    
        /**
         * Loads LOD tree mesh given mesh id.
         * @param meshId The mesh id to be loaded.
         * @return The mesh corresponding to the id or null if mesh could not be loaded.
         */
        virtual std::shared_ptr<nml::Mesh> loadMesh(long long meshId) = 0;
    
        /**
         * Loads LOD tree texture given texture id and mip level.
         * @param textureId The id of a texture to be loaded.
         * @param level The mip level of the texture to loaded.
         * @return The texture corresponding to the id/level or null pointer if texture could not be loaded.
         */
        virtual std::shared_ptr<nml::Texture> loadTexture(long long textureId, int level) = 0;
    
    protected:
        explicit NMLModelLODTreeDataSource(const std::shared_ptr<Projection>& projection);
    
        std::shared_ptr<Projection> _projection;
    
        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
