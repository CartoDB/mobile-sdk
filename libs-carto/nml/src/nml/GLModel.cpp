#include "GLModel.h"
#include "GLMesh.h"
#include "GLMeshInstance.h"
#include "GLTexture.h"
#include "GLShaderManager.h"
#include "Package.h"

namespace carto { namespace nml {

    GLModel::GLModel(const Model& model) :
        _meshMap(),
        _textureMap(),
        _meshInstanceList()
    {
        // Get bounds
        const Vector3& minBounds = model.bounds().min();
        const Vector3& maxBounds = model.bounds().max();
        _bounds = cglib::bbox3<float>(cglib::vec3<float>(minBounds.x(), minBounds.y(), minBounds.z()), cglib::vec3<float>(maxBounds.x(), maxBounds.y(), maxBounds.z()));
    
        // Build map from texture ids to GL textures
        for (int i = 0; i < model.textures_size(); i++) {
            auto texture = std::make_shared<Texture>(model.textures(i));
            auto glTexture = std::make_shared<GLTexture>(texture);
            _textureMap[texture->id()] = glTexture;
        }
        
        // Build map from meshes to GL mesh objects
        std::map<std::string, std::shared_ptr<GLMesh>> meshMap;
        for (int i = 0; i < model.meshes_size(); i++) {
            const Mesh& mesh = model.meshes(i);
            auto glMesh = std::make_shared<GLMesh>(model.meshes(i));
            meshMap[mesh.id()] = glMesh;
            _meshMap[mesh.id()] = { glMesh, std::shared_ptr<MeshOp>() };
        }
    
        // Create mesh instances
        for (int i = 0; i < model.mesh_instances_size(); i++) {
            const MeshInstance& meshInstance = model.mesh_instances(i);
            auto glMeshInstance = std::make_shared<GLMeshInstance>(meshInstance, meshMap, _textureMap);
            _meshInstanceList.push_back(glMeshInstance);
        }
    }
    
    void GLModel::create(GLShaderManager& shaderManager) {
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
    
    void GLModel::dispose() {
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
    
    void GLModel::replaceMesh(const std::string& id, const std::shared_ptr<GLMesh>& mesh) {
        replaceMesh(id, mesh, std::shared_ptr<MeshOp>());
    }
    
    void GLModel::replaceMesh(const std::string& id, const std::shared_ptr<GLMesh>& mesh, const std::shared_ptr<MeshOp>& meshOp) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        auto meshIt = _meshMap.find(id);
        if (meshIt != _meshMap.end()) {
            if (meshIt->second.first == mesh && meshIt->second.second == meshOp) {
                return;
            }
        }
    
        _meshMap[id] = { mesh, meshOp };
        
        std::shared_ptr<GLMesh> finalMesh = mesh;
        if (meshOp) {
            finalMesh = std::make_shared<GLMesh>(*mesh, *meshOp);
        }
        
        for (const std::shared_ptr<GLMeshInstance>& meshInstance : _meshInstanceList) {
            meshInstance->replaceMesh(id, finalMesh);
        }
    }
    
    void GLModel::replaceTexture(const std::string& id, const std::shared_ptr<GLTexture>& texture) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        auto texIt = _textureMap.find(id);
        if (texIt != _textureMap.end()) {
            if (texIt->second == texture) {
                return;
            }
        }
    
        _textureMap[id] = texture;

        for (const std::shared_ptr<GLMeshInstance>& meshInstance : _meshInstanceList) {
            meshInstance->replaceTexture(id, texture);
        }
    }
    
    void GLModel::draw(const RenderState& renderState)  {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<GLMeshInstance>& meshInstance : _meshInstanceList) {
            meshInstance->draw(renderState);
        }
    }
    
    void GLModel::calculateRayIntersections(const cglib::ray3<double>& ray, std::vector<RayIntersection>& intersections) const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (const std::shared_ptr<GLMeshInstance>& meshInstance : _meshInstanceList) {
            meshInstance->calculateRayIntersections(ray, intersections);
        }
    }
    
    cglib::bbox3<float> GLModel::getBounds() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bounds;
    }
    
    int GLModel::getDrawCallCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        int count = 0;
        for (const std::shared_ptr<GLMeshInstance>& meshInstance : _meshInstanceList) {
            count += meshInstance->getDrawCallCount();
        }
        return count;
    }
    
    int GLModel::getTotalGeometrySize() const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        int size = 0;
        for (auto it = _meshMap.begin(); it != _meshMap.end(); it++) {
            size += it->second.first->getTotalGeometrySize();
        }
        return size;
    }
    
} }
