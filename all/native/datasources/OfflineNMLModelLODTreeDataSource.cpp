#if defined(_CARTO_NMLMODELLODTREE_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "OfflineNMLModelLODTreeDataSource.h"
#include "components/Exceptions.h"
#include "graphics/ViewState.h"
#include "projections/ProjectionSurface.h"
#include "renderers/components/CullState.h"
#include "projections/EPSG3857.h"
#include "utils/Log.h"

#include <cglib/bbox.h>
#include <cglib/frustum3.h>

#include <nml/Package.h>

#include <sqlite3pp.h>

namespace carto {

    OfflineNMLModelLODTreeDataSource::OfflineNMLModelLODTreeDataSource(const std::string& path) :
        NMLModelLODTreeDataSource(std::make_shared<EPSG3857>()),
        _database(std::make_unique<sqlite3pp::database>())
    {
        if (_database->connect_v2(path.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
            throw FileException("Failed to open database", path);
        }
        _database->execute("PRAGMA temp_store=MEMORY");
    }
    
    OfflineNMLModelLODTreeDataSource::~OfflineNMLModelLODTreeDataSource() {
    }

    MapBounds OfflineNMLModelLODTreeDataSource::getDataExtent() const {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_database) {
            Log::Error("NMLModelLODTreeDataSource::getDataExtent: Failed to load tiles, could not connect to database");
            return MapBounds();
        }
    
        MapBounds dataExtent;
        sqlite3pp::query query(*_database, "SELECT mapbounds_x0, mapbounds_y0, mapbounds_x1, mapbounds_y1 FROM MapTiles");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            double mapBoundsX0 = (*qit).get<double>(0);
            double mapBoundsY0 = (*qit).get<double>(1);
            double mapBoundsX1 = (*qit).get<double>(2);
            double mapBoundsY1 = (*qit).get<double>(3);

            dataExtent.expandToContain(MapPos(mapBoundsX0, mapBoundsY0));
            dataExtent.expandToContain(MapPos(mapBoundsX1, mapBoundsY1));
        }
        query.finish();
        return dataExtent;
    }
    
    std::vector<NMLModelLODTreeDataSource::MapTile> OfflineNMLModelLODTreeDataSource::loadMapTiles(const std::shared_ptr<CullState>& cullState) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_database) {
            Log::Error("NMLModelLODTreeDataSource::loadMapTiles: Failed to load tiles, could not connect to database");
            return std::vector<MapTile>();
        }
    
        MapBounds bounds = cullState->getProjectionEnvelope(_projection).getBounds();
        
        sqlite3pp::query query(*_database, "SELECT id, modellodtree_id, mappos_x, mappos_y, groundheight, mapbounds_x0, mapbounds_y0, mapbounds_x1, mapbounds_y1 FROM MapTiles WHERE ((mapbounds_x1>=:x0 AND mapbounds_x0<=:x1) OR (mapbounds_x1>=:x0 + :width AND mapbounds_x0<=:x1 + :width) OR (mapbounds_x1>=:x0 - :width AND mapbounds_x0<=:x1 - :width)) AND (mapbounds_y1>=:y0 AND mapbounds_y0<=:y1)");
        query.bind(":x0", bounds.getMin().getX());
        query.bind(":y0", bounds.getMin().getY());
        query.bind(":x1", bounds.getMax().getX());
        query.bind(":y1", bounds.getMax().getY());
        query.bind(":width", _projection->getBounds().getDelta().getX());

        std::vector<MapTile> mapTiles;
        
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            long long mapTileId = (*qit).get<std::uint64_t>(0);
            
            long long modelLODTreeId = (*qit).get<std::uint64_t>(1);
            double mapPosX = (*qit).get<double>(2);
            double mapPosY = (*qit).get<double>(3);
            double mapPosZ = (*qit).get<double>(4);
            double mapBoundsX0 = (*qit).get<double>(5);
            double mapBoundsY0 = (*qit).get<double>(6);
            double mapBoundsX1 = (*qit).get<double>(7);
            double mapBoundsY1 = (*qit).get<double>(8);

            const std::shared_ptr<ProjectionSurface>& projectionSurface = cullState->getViewState().getProjectionSurface();

            cglib::bbox3<double> bbox = cglib::bbox3<double>::smallest();
            for (int i = 0; i < 8; i++) {
                MapPos mapPos(i & 1 ? mapBoundsX1 : mapBoundsX0, i & 2 ? mapBoundsY1 : mapBoundsY0, i & 4 ? mapPosZ + MAX_HEIGHT : mapPosZ + MIN_HEIGHT);
                bbox.add(projectionSurface->calculatePosition(_projection->toInternal(mapPos)));
            }
            if (!cullState->getViewState().getFrustum().inside(bbox)) {
                continue;
            }
    
            MapTile mapTile(mapTileId, MapPos(mapPosX, mapPosY, mapPosZ), modelLODTreeId);
            mapTiles.push_back(mapTile);
        }
        query.finish();
        return mapTiles;
    }
    
    std::shared_ptr<NMLModelLODTree> OfflineNMLModelLODTreeDataSource::loadModelLODTree(const MapTile& mapTile) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_database) {
            Log::Error("OfflineNMLModelLODTreeDataSource::loadModelLODTree: Failed to load LOD tree, could not connect to database");
            return std::shared_ptr<NMLModelLODTree>();
        }
    
        sqlite3pp::query query(*_database, "SELECT id, nmlmodellodtree FROM ModelLODTrees WHERE id=:id");
        query.bind(":id", static_cast<std::uint64_t>(mapTile.modelLODTreeId));
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            long long modelLODTreeId = (*qit).get<std::uint64_t>(0);
            std::size_t nmlModelLODTreeSize = (*qit).column_bytes(1);
            const void* nmlModelLODTreeData = (*qit).get<const void*>(1);
            std::shared_ptr<nml::ModelLODTree> sourceModelLODTree = std::make_shared<nml::ModelLODTree>(protobuf::message(nmlModelLODTreeData, nmlModelLODTreeSize));
    
            sqlite3pp::query queryProxyBindings(*_database, "SELECT * FROM ModelInfo WHERE modellodtree_id=:modellodtree_id");
            queryProxyBindings.bind(":modellodtree_id", static_cast<std::uint64_t>(modelLODTreeId));
            NMLModelLODTree::ProxyMap proxyMap;
            for (auto qitProxyBindings = queryProxyBindings.begin(); qitProxyBindings != queryProxyBindings.end(); qitProxyBindings++) {
                int modelId = -1;
                MapPos mapPos(0, 0, 0);
                std::map<std::string, std::string> metaData;
                for (int i = 0; i < queryProxyBindings.column_count(); i++) {
                    std::string columnName = queryProxyBindings.column_name(i);
                    if (columnName == "model_id") {
                        modelId = (*qitProxyBindings).get<int32_t>(i);
                    } else if (columnName == "mappos_x") {
                        mapPos.setX((*qitProxyBindings).get<double>(i));
                    } else if (columnName == "mappos_y") {
                        mapPos.setY((*qitProxyBindings).get<double>(i));
                    } else if (columnName == "groundheight") {
                        mapPos.setZ((*qitProxyBindings).get<double>(i));
                    } else if (columnName != "modellodtree_id" && columnName != "global_id") {
                        metaData[columnName] = (*qitProxyBindings).get<const char*>(i);
                    }
                }
    
                proxyMap.emplace(modelId, NMLModelLODTree::Proxy(modelId, mapPos, metaData));
            }
            queryProxyBindings.finish();
    
            NMLModelLODTree::MeshBindingsMap meshBindingsMap;
            try {
                sqlite3pp::query queryMeshBindings(*_database, "SELECT node_id, local_id, mesh_id, nmlmeshop FROM ModelLODTreeNodeMeshes WHERE modellodtree_id=:modellodtree_id");
                queryMeshBindings.bind(":modellodtree_id", static_cast<std::uint64_t>(modelLODTreeId));
                for (auto qitMeshBindings = queryMeshBindings.begin(); qitMeshBindings != queryMeshBindings.end(); qitMeshBindings++) {
                    int nodeId = (*qitMeshBindings).get<std::uint32_t>(0);
                    std::string localId = (*qitMeshBindings).get<const char*>(1);
                    long long meshId = (*qitMeshBindings).get<std::uint64_t>(2);
                    std::size_t nmlMeshOpSize = (*qitMeshBindings).column_bytes(3);
                    const void* nmlMeshOpData = (*qitMeshBindings).get<const void*>(3);
                    if (nmlMeshOpSize > 0) {
                        std::shared_ptr<nml::MeshOp> meshOp = std::make_shared<nml::MeshOp>(protobuf::message(nmlMeshOpData, nmlMeshOpSize));
                        meshBindingsMap[nodeId].push_back(NMLModelLODTree::MeshBinding(meshId, localId, meshOp));
                    } else {
                        meshBindingsMap[nodeId].push_back(NMLModelLODTree::MeshBinding(meshId, localId));
                    }
                }
                queryMeshBindings.finish();
            }
            catch (const sqlite3pp::database_error& ) {
                Log::Error("OfflineNMLModelLODTreeDataSource: Mesh query failed. Legacy database without 'nmlmeshop' column?");
            }
    
            sqlite3pp::query queryTexBindings(*_database, "SELECT node_id, local_id, texture_id, level FROM ModelLODTreeNodeTextures WHERE modellodtree_id=:modellodtree_id");
            queryTexBindings.bind(":modellodtree_id", static_cast<std::uint64_t>(modelLODTreeId));
            NMLModelLODTree::TextureBindingsMap textureBindingsMap;
            for (auto qitTexBindings = queryTexBindings.begin(); qitTexBindings != queryTexBindings.end(); qitTexBindings++) {
                int nodeId = (*qitTexBindings).get<std::uint32_t>(0);
                std::string localId = (*qitTexBindings).get<const char*>(1);
                long long textureId = (*qitTexBindings).get<std::uint64_t>(2);
                int level = (*qitTexBindings).get<std::uint32_t>(3);
                textureBindingsMap[nodeId].push_back(NMLModelLODTree::TextureBinding(textureId, level, localId));
            }
            queryTexBindings.finish();
    
            std::shared_ptr<NMLModelLODTree> modelLODTree = std::make_shared<NMLModelLODTree>(modelLODTreeId, mapTile.mapPos, _projection, sourceModelLODTree, proxyMap, meshBindingsMap, textureBindingsMap);
            return modelLODTree;
        }
        query.finish();
        return std::shared_ptr<NMLModelLODTree>();
    }
    
    std::shared_ptr<nml::Mesh> OfflineNMLModelLODTreeDataSource::loadMesh(long long meshId) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_database) {
            Log::Error("OfflineNMLModelLODTreeDataSource::loadMesh: Failed to load mesh, could not connect to database");
            return std::shared_ptr<nml::Mesh>();
        }
    
        sqlite3pp::query query(*_database, "SELECT nmlmesh FROM Meshes WHERE id=:source_id");
        query.bind(":source_id", static_cast<std::uint64_t>(meshId));
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::size_t nmlMeshSize = (*qit).column_bytes(0);
            const void* nmlMeshData = (*qit).get<const void*>(0);
            std::shared_ptr<nml::Mesh> mesh = std::make_shared<nml::Mesh>(protobuf::message(nmlMeshData, nmlMeshSize));
            return mesh;
        }
        query.finish();
        return std::shared_ptr<nml::Mesh>();
    }
    
    std::shared_ptr<nml::Texture> OfflineNMLModelLODTreeDataSource::loadTexture(long long textureId, int level) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_database) {
            Log::Error("OfflineNMLModelLODTreeDataSource::loadTexture: Failed to load texture, could not connect to database");
            return std::shared_ptr<nml::Texture>();
        }
    
        sqlite3pp::query query(*_database, "SELECT nmltexture FROM Textures WHERE id=:source_id AND textures.level=:level ORDER BY textures.level ASC");
        query.bind(":source_id", static_cast<std::uint64_t>(textureId));
        query.bind(":level", level);
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::size_t nmlTextureSize = (*qit).column_bytes(0);
            const void* nmlTextureData = (*qit).get<const void*>(0);
            std::shared_ptr<nml::Texture> texture = std::make_shared<nml::Texture>(protobuf::message(nmlTextureData, nmlTextureSize));
            return texture;
        }
        query.finish();
        return std::shared_ptr<nml::Texture>();
    }

    const float OfflineNMLModelLODTreeDataSource::MIN_HEIGHT = 0.0f;
    const float OfflineNMLModelLODTreeDataSource::MAX_HEIGHT = 1000.0f;
    
}

#endif
