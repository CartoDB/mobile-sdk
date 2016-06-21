#include "Model.h"
#include "Mesh.h"
#include "MeshInstance.h"
#include "Texture.h"
#include "ShaderManager.h"

#include "nmlpackage/NMLPackage.pb.h"

namespace carto { namespace nmlgl {

    Model::Model(const nml::Model& model) :
        _meshMap(),
        _textureMap(),
        _meshInstanceList()
    {
        // Get bounds
        const nml::Vector3& minBounds = model.bounds().min();
        const nml::Vector3& maxBounds = model.bounds().max();
        _bounds = cglib::bbox3<float>(cglib::vec3<float>(minBounds.x(), minBounds.y(), minBounds.z()), cglib::vec3<float>(maxBounds.x(), maxBounds.y(), maxBounds.z()));
    
        // Build map from texture ids to GL textures
        for (int i = 0; i < model.textures_size(); i++) {
            auto texture = std::make_shared<nml::Texture>(model.textures(i));
            auto glTexture = std::make_shared<Texture>(texture);
            _textureMap[texture->id()] = glTexture;
        }
        
        // Build map from meshes to GL mesh objects
        std::map<std::string, std::shared_ptr<Mesh>> meshMap;
        for (int i = 0; i < model.meshes_size(); i++) {
            const nml::Mesh& mesh = model.meshes(i);
            auto glMesh = std::make_shared<Mesh>(model.meshes(i));
            meshMap[mesh.id()] = glMesh;
            _meshMap[mesh.id()] = std::make_pair(glMesh, std::shared_ptr<nml::MeshOp>());
        }
    
        // Create mesh instances
        for (int i = 0; i < model.mesh_instances_size(); i++) {
            const nml::MeshInstance& meshInstance = model.mesh_instances(i);
            auto glMeshInstance = std::make_shared<MeshInstance>(meshInstance, meshMap, _textureMap);
            _meshInstanceList.push_back(glMeshInstance);
        }
    }
    
    void Model::create(ShaderManager& shaderManager) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (auto it = _meshMap.begin(); it != _meshMap.end(); it++) {
            it->second.first->create();
        }
    
        for (auto it = _textureMap.begin(); it != _textureMap.end(); it++) {
            it->second->create();
        }

        for (auto it = _meshInstanceList.begin(); it != _meshInstanceList.end(); it++) {
            (*it)->create(shaderManager);
        }
    }
    
    void Model::dispose() {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (auto it = _meshInstanceList.begin(); it != _meshInstanceList.end(); it++) {
            (*it)->dispose();
        }

        for (auto it = _meshMap.begin(); it != _meshMap.end(); it++) {
            it->second.first->dispose();
        }
    
        for (auto it = _textureMap.begin(); it != _textureMap.end(); it++) {
            it->second->dispose();
        }
    }
    
    void Model::replaceMesh(const std::string& id, const std::shared_ptr<Mesh>& glMesh) {
        replaceMesh(id, glMesh, std::shared_ptr<nml::MeshOp>());
    }
    
    void Model::replaceMesh(const std::string& id, const std::shared_ptr<Mesh>& glMesh, const std::shared_ptr<nml::MeshOp>& meshOp) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        auto meshIt = _meshMap.find(id);
        if (meshIt != _meshMap.end()) {
            if (meshIt->second.first == glMesh && meshIt->second.second == meshOp) {
                return;
            }
        }
    
        _meshMap[id] = std::make_pair(glMesh, meshOp);
        
        std::shared_ptr<Mesh> glFinalMesh = glMesh;
        if (meshOp) {
            glFinalMesh = std::make_shared<nmlgl::Mesh>(*glMesh, *meshOp);
        }
        
        for (const std::shared_ptr<MeshInstance>& meshInstance : _meshInstanceList) {
            meshInstance->replaceMesh(id, glFinalMesh);
        }
    }
    
    void Model::replaceTexture(const std::string& id, const std::shared_ptr<Texture>& glTexture) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        auto texIt = _textureMap.find(id);
        if (texIt != _textureMap.end()) {
            if (texIt->second == glTexture) {
                return;
            }
        }
    
        _textureMap[id] = glTexture;

        for (const std::shared_ptr<MeshInstance>& meshInstance : _meshInstanceList) {
            meshInstance->replaceTexture(id, glTexture);
        }
    }
    
    void Model::draw(const RenderState& renderState)  {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<MeshInstance>& meshInstance : _meshInstanceList) {
            meshInstance->draw(renderState);
        }
    }
    
    void Model::calculateRayIntersections(const Ray& ray, std::vector<RayIntersection>& intersections) const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<MeshInstance>& meshInstance : _meshInstanceList) {
            meshInstance->calculateRayIntersections(ray, intersections);
        }
    }
    
    cglib::bbox3<float> Model::getBounds() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bounds;
    }
    
    int Model::getDrawCallCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        int count = 0;
        for (const std::shared_ptr<MeshInstance>& meshInstance : _meshInstanceList) {
            count += meshInstance->getDrawCallCount();
        }
        return count;
    }
    
    int Model::getTotalGeometrySize() const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        int size = 0;
        for (auto it = _meshMap.begin(); it != _meshMap.end(); it++) {
            size += it->second.first->getTotalGeometrySize();
        }
        return size;
    }
    
} }
