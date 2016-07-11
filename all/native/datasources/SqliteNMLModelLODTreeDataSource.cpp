#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "SqliteNMLModelLODTreeDataSource.h"
#include "components/Exceptions.h"
#include "renderers/components/CullState.h"
#include "projections/EPSG3857.h"
#include "utils/Log.h"

#include <nml/Package.h>

#include <sqlite3pp.h>

#include <cglib/frustum3.h>

namespace carto {

    SqliteNMLModelLODTreeDataSource::SqliteNMLModelLODTreeDataSource(const std::string& fileName) :
        NMLModelLODTreeDataSource(std::make_shared<EPSG3857>()),
        _db()
    {
        try {
            _db.reset(new sqlite3pp::database(fileName.c_str()));
            _db->execute("PRAGMA encoding='UTF-8'");
        } catch (const std::exception& e) {
            throw FileException("Failed to open database", fileName);
        }
    }
    
    SqliteNMLModelLODTreeDataSource::~SqliteNMLModelLODTreeDataSource() {
    }
    
    std::vector<NMLModelLODTreeDataSource::MapTile> SqliteNMLModelLODTreeDataSource::loadMapTiles(const std::shared_ptr<CullState>& cullState) {
        typedef cglib::vec3<double> Point;
        typedef cglib::frustum3<double> Frustum;
        typedef cglib::bbox3<double> BoundingBox;
    
        static const float MAX_HEIGHT = 1000.0f;
    
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_db) {
            Log::Error("NMLModelLODTreeDataSource::loadMapTiles: Failed to load tiles, couldn't connect to database.");
            return std::vector<MapTile>();
        }
    
        std::vector<MapTile> mapTiles;
        
        Frustum frustum(cglib::gl_projection_frustum(cullState->getViewState().getModelviewProjectionMat()));
    
        MapBounds bounds(_projection->fromInternal(cullState->getEnvelope().getBounds().getMin()), _projection->fromInternal(cullState->getEnvelope().getBounds().getMax()));
        
        sqlite3pp::query query(*_db, "SELECT id, modellodtree_id, mappos_x, mappos_y, groundheight, mapbounds_x0, mapbounds_y0, mapbounds_x1, mapbounds_y1 FROM MapTiles WHERE ((mapbounds_x1>=:x0 AND mapbounds_x0<=:x1) OR (mapbounds_x1>=:x0 + :width AND mapbounds_x0<=:x1 + :width) OR (mapbounds_x1>=:x0 - :width AND mapbounds_x0<=:x1 - :width)) AND (mapbounds_y1>=:y0 AND mapbounds_y0<=:y1)");
        query.bind(":x0", bounds.getMin().getX());
        query.bind(":y0", bounds.getMin().getY());
        query.bind(":x1", bounds.getMax().getX());
        query.bind(":y1", bounds.getMax().getY());
        query.bind(":width", _projection->getBounds().getDelta().getX());
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            long long mapTileId = (*qit).get<uint64_t>(0);
            
            long long modelLODTreeId = (*qit).get<uint64_t>(1);
            double mapPosX = (*qit).get<double>(2);
            double mapPosY = (*qit).get<double>(3);
            double mapPosZ = (*qit).get<double>(4);
            double mapBoundsX0 = (*qit).get<double>(5);
            double mapBoundsY0 = (*qit).get<double>(6);
            double mapBoundsX1 = (*qit).get<double>(7);
            double mapBoundsY1 = (*qit).get<double>(8);
    
            MapPos intMinPos(_projection->toInternal(MapPos(mapBoundsX0, mapBoundsY0, mapPosZ)));
            MapPos intMaxPos(_projection->toInternal(MapPos(mapBoundsX1, mapBoundsY1, mapPosZ + MAX_HEIGHT)));
            BoundingBox bbox(Point(intMinPos.getX(), intMinPos.getY(), intMinPos.getZ()), Point(intMaxPos.getX(), intMaxPos.getY(), intMaxPos.getZ()));
            if (!frustum.inside(bbox)) {
                continue;
            }
    
            MapTile mapTile(mapTileId, MapPos(mapPosX, mapPosY, mapPosZ), modelLODTreeId);
            mapTiles.push_back(mapTile);
        }
        query.finish();
        return mapTiles;
    }
    
    std::shared_ptr<NMLModelLODTree> SqliteNMLModelLODTreeDataSource::loadModelLODTree(const MapTile& mapTile) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_db) {
            Log::Error("SqliteNMLModelLODTreeDataSource::loadModelLODTree: Failed to load LOD tree, couldn't connect to database.");
            return std::shared_ptr<NMLModelLODTree>();
        }
    
        sqlite3pp::query query(*_db, "SELECT id, LENGTH(nmlmodellodtree), nmlmodellodtree FROM ModelLODTrees WHERE id=:id");
        query.bind(":id", static_cast<uint64_t>(mapTile.modelLODTreeId));
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            long long modelLODTreeId = (*qit).get<uint64_t>(0);
            std::size_t nmlModelLODTreeSize = (*qit).get<uint32_t>(1);
            const void * nmlModelLODTreeData = (*qit).get<const void *>(2);
            std::shared_ptr<nml::ModelLODTree> sourceModelLODTree = std::make_shared<nml::ModelLODTree>(protobuf::message(nmlModelLODTreeData, nmlModelLODTreeSize));
    
            sqlite3pp::query queryProxyBindings(*_db, "SELECT * FROM ModelInfo WHERE modellodtree_id=:modellodtree_id");
            queryProxyBindings.bind(":modellodtree_id", static_cast<uint64_t>(modelLODTreeId));
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
                        metaData[columnName] = (*qitProxyBindings).get<const char *>(i);
                    }
                }
    
                proxyMap.emplace(modelId, NMLModelLODTree::Proxy(modelId, mapPos, metaData));
            }
            queryProxyBindings.finish();
    
            NMLModelLODTree::MeshBindingsMap meshBindingsMap;
            try {
                sqlite3pp::query queryMeshBindings(*_db, "SELECT node_id, local_id, mesh_id, LENGTH(nmlmeshop), nmlmeshop FROM ModelLODTreeNodeMeshes WHERE modellodtree_id=:modellodtree_id");
                queryMeshBindings.bind(":modellodtree_id", static_cast<uint64_t>(modelLODTreeId));
                for (auto qitMeshBindings = queryMeshBindings.begin(); qitMeshBindings != queryMeshBindings.end(); qitMeshBindings++) {
                    int nodeId = (*qitMeshBindings).get<uint32_t>(0);
                    std::string localId = (*qitMeshBindings).get<const char *>(1);
                    long long meshId = (*qitMeshBindings).get<uint64_t>(2);
                    std::size_t nmlMeshOpSize = (*qitMeshBindings).get<uint32_t>(3);
                    const void * nmlMeshOpData = (*qitMeshBindings).get<const void *>(4);
                    if (nmlMeshOpSize > 0) {
                        std::shared_ptr<nml::MeshOp> meshOp = std::make_shared<nml::MeshOp>(protobuf::message(nmlMeshOpData, nmlMeshOpSize));
                        meshBindingsMap[nodeId].push_back(NMLModelLODTree::MeshBinding(meshId, localId, meshOp));
                    } else {
                        meshBindingsMap[nodeId].push_back(NMLModelLODTree::MeshBinding(meshId, localId));
                    }
                }
                queryMeshBindings.finish();
            } catch (const sqlite3pp::database_error& ) {
                Log::Error("SqliteNMLModelLODTreeDataSource: Mesh query failed. Legacy database without 'nmlmeshop' column?");
            }
    
            sqlite3pp::query queryTexBindings(*_db, "SELECT node_id, local_id, texture_id, level FROM ModelLODTreeNodeTextures WHERE modellodtree_id=:modellodtree_id");
            queryTexBindings.bind(":modellodtree_id", static_cast<uint64_t>(modelLODTreeId));
            NMLModelLODTree::TextureBindingsMap textureBindingsMap;
            for (auto qitTexBindings = queryTexBindings.begin(); qitTexBindings != queryTexBindings.end(); qitTexBindings++) {
                int nodeId = (*qitTexBindings).get<uint32_t>(0);
                std::string localId = (*qitTexBindings).get<const char *>(1);
                long long textureId = (*qitTexBindings).get<uint64_t>(2);
                int level = (*qitTexBindings).get<uint32_t>(3);
                textureBindingsMap[nodeId].push_back(NMLModelLODTree::TextureBinding(textureId, level, localId));
            }
            queryTexBindings.finish();
    
            std::shared_ptr<NMLModelLODTree> modelLODTree = std::make_shared<NMLModelLODTree>(modelLODTreeId, mapTile.mapPos, _projection, sourceModelLODTree, proxyMap, meshBindingsMap, textureBindingsMap);
            return modelLODTree;
        }
        query.finish();
        return std::shared_ptr<NMLModelLODTree>();
    }
    
    std::shared_ptr<nml::Mesh> SqliteNMLModelLODTreeDataSource::loadMesh(long long meshId) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_db) {
            Log::Error("SqliteNMLModelLODTreeDataSource::loadMesh: Failed to load mesh, couldn't connect to database.");
            return std::shared_ptr<nml::Mesh>();
        }
    
        sqlite3pp::query query(*_db, "SELECT LENGTH(nmlmesh), nmlmesh FROM Meshes WHERE id=:source_id");
        query.bind(":source_id", static_cast<uint64_t>(meshId));
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::size_t nmlMeshSize = (*qit).get<uint32_t>(0);
            const void * nmlMeshData = (*qit).get<const void *>(1);
            std::shared_ptr<nml::Mesh> mesh = std::make_shared<nml::Mesh>(protobuf::message(nmlMeshData, nmlMeshSize));
            return mesh;
        }
        query.finish();
        return std::shared_ptr<nml::Mesh>();
    }
    
    std::shared_ptr<nml::Texture> SqliteNMLModelLODTreeDataSource::loadTexture(long long textureId, int level) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        if (!_db) {
            Log::Error("SqliteNMLModelLODTreeDataSource::loadTexture: Failed to load texture, couldn't connect to database.");
            return std::shared_ptr<nml::Texture>();
        }
    
        sqlite3pp::query query(*_db, "SELECT LENGTH(nmltexture), nmltexture FROM Textures WHERE id=:source_id AND textures.level=:level ORDER BY textures.level ASC");
        query.bind(":source_id", static_cast<uint64_t>(textureId));
        query.bind(":level", level);
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::size_t nmlTextureSize = (*qit).get<uint32_t>(0);
            const void * nmlTextureData = (*qit).get<const void *>(1);
            std::shared_ptr<nml::Texture> texture = std::make_shared<nml::Texture>(protobuf::message(nmlTextureData, nmlTextureSize));
            return texture;
        }
        query.finish();
        return std::shared_ptr<nml::Texture>();
    }
    
}

#endif
