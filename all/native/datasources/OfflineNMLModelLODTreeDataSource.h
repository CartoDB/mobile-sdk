/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_OFFLINENMLMODELLODTREEDATASOURCE_H_
#define _CARTO_OFFLINENMLMODELLODTREEDATASOURCE_H_

#if defined(_CARTO_NMLMODELLODTREE_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "datasources/NMLModelLODTreeDataSource.h"

namespace sqlite3pp {
    class database;
}

namespace carto {

    /**
     * A sqlite database based data source for NML model LOD trees. The database must be created using
     * custom toolkit from Carto that supports several input formats like KMZ or GeoJSON.
     */
    class OfflineNMLModelLODTreeDataSource : public NMLModelLODTreeDataSource {
    public:
        /**
         * Constructs a OfflineNMLModelLODTreeDataSource object.
         * @param path The path to the sqlite database file.
         * @throws std::exception If the file could not be opened.
         */
        explicit OfflineNMLModelLODTreeDataSource(const std::string& path);
        virtual ~OfflineNMLModelLODTreeDataSource();

        virtual std::vector<MapTile> loadMapTiles(const std::shared_ptr<CullState>& cullState);
        virtual std::shared_ptr<NMLModelLODTree> loadModelLODTree(const MapTile& mapTile);
        virtual std::shared_ptr<nml::Mesh> loadMesh(long long meshId);
        virtual std::shared_ptr<nml::Texture> loadTexture(long long textureId, int level);

    private:
        std::unique_ptr<sqlite3pp::database> _db;
    };

}

#endif

#endif
