#include "MeshInstance.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Submesh.h"
#include "ShaderManager.h"

#include "nmlpackage/NMLPackage.pb.h"

namespace carto { namespace nmlgl {

    MeshInstance::MeshInstance(const nml::MeshInstance& meshInstance, const std::map<std::string, std::shared_ptr<Mesh>>& meshMap, const std::map<std::string, std::shared_ptr<Texture>>& textureMap) :
        _transformEnabled(false)
    {
        _meshId = meshInstance.mesh_id();
    
        // Look up mesh
        auto meshIt = meshMap.find(meshInstance.mesh_id());
        if (meshIt != meshMap.end()) {
            _mesh = meshIt->second;
        }
    
        // Set node transformation matrix
        _transformEnabled = meshInstance.has_transform();
        if (_transformEnabled) {
            const nml::Matrix4& transform = meshInstance.transform();
            _transformMatrix(0, 0) = transform.m00();
            _transformMatrix(1, 0) = transform.m10();
            _transformMatrix(2, 0) = transform.m20();
            _transformMatrix(3, 0) = transform.m30();
    
            _transformMatrix(0, 1) = transform.m01();
            _transformMatrix(1, 1) = transform.m11();
            _transformMatrix(2, 1) = transform.m21();
            _transformMatrix(3, 1) = transform.m31();
    
            _transformMatrix(0, 2) = transform.m02();
            _transformMatrix(1, 2) = transform.m12();
            _transformMatrix(2, 2) = transform.m22();
            _transformMatrix(3, 2) = transform.m32();
    
            _transformMatrix(0, 3) = transform.m03();
            _transformMatrix(1, 3) = transform.m13();
            _transformMatrix(2, 3) = transform.m23();
            _transformMatrix(3, 3) = transform.m33();
        } else {
            _transformMatrix = cglib::mat4x4<float>::identity();
        }

        // Create inverse-transpose of the transform matrix for normals
        _invTransTransformMatrix = cglib::transpose(cglib::inverse(_transformMatrix));
    
        // Create material map
        for (int i = 0; i < meshInstance.materials_size(); i++) {
            const nml::Material& material = meshInstance.materials(i);
            auto glMaterial = std::make_shared<Material>(material, textureMap);
            _materialMap[material.id()] = glMaterial;
        }
    }

    void MeshInstance::create(ShaderManager& shaderManager) {
        for (auto it = _materialMap.begin(); it != _materialMap.end(); it++) {
            it->second->create(shaderManager);
        }
    }

    void MeshInstance::dispose() {
        for (auto it = _materialMap.begin(); it != _materialMap.end(); it++) {
            it->second->dispose();
        }
    }

    void MeshInstance::replaceMesh(const std::string& meshId, const std::shared_ptr<Mesh>& glMesh) {
        if (_meshId == meshId) {
            _mesh = glMesh;
        }
    }
    
    void MeshInstance::replaceTexture(const std::string& textureId, const std::shared_ptr<Texture>& glTexture) {
        for (auto it = _materialMap.begin(); it != _materialMap.end(); it++) {
            it->second->replaceTexture(textureId, glTexture);
        }
    }
    
    void MeshInstance::draw(const RenderState& renderState) {
        if (!_mesh) {
            return;
        }
    
        cglib::mat4x4<float> mvMatrix = renderState.mvMatrix;
        cglib::mat4x4<float> invTransMVMatrix = renderState.invTransMVMatrix;
        if (_transformEnabled) {
            mvMatrix = mvMatrix * _transformMatrix;
            invTransMVMatrix = invTransMVMatrix * _invTransTransformMatrix;
        }

        for (const std::shared_ptr<Submesh>& submesh : _mesh->getSubmeshList()) {
            auto materialIt = _materialMap.find(submesh->getMaterialId());
            if (materialIt != _materialMap.end()) {
                const std::shared_ptr<Material>& material = materialIt->second;

                material->bind(renderState, mvMatrix, invTransMVMatrix);
                submesh->draw(renderState);
            }
        }
    }
    
    void MeshInstance::calculateRayIntersections(const Ray& ray, std::vector<RayIntersection>& intersections) const {
        if (!_mesh) {
            return;
        }
    
        Ray rayTransformed = ray;
        if (_transformEnabled) {
            cglib::mat4x4<double> invTransformMatrix = cglib::inverse(cglib::mat4x4<double>::convert(_transformMatrix));
            cglib::vec3<double> originTransformed = cglib::transform_point(ray.origin, invTransformMatrix);
            cglib::vec3<double> dirTransformed = cglib::transform_point(ray.origin + ray.dir, invTransformMatrix) - originTransformed;
            rayTransformed = Ray(originTransformed, dirTransformed);
        }

        for (const std::shared_ptr<Submesh>& submesh : _mesh->getSubmeshList()) {
            auto materialIt = _materialMap.find(submesh->getMaterialId());
            if (materialIt != _materialMap.end()) {
                const std::shared_ptr<Material>& material = materialIt->second;

                std::vector<RayIntersection> submeshIntersections;
                submesh->calculateRayIntersections(rayTransformed, submeshIntersections);
    
                for (std::size_t i = 0; i < submeshIntersections.size(); i++) {
                    RayIntersection intersection = submeshIntersections[i];
                    if (material->getCulling() != nml::Material::NONE) {
                        double sign = material->getCulling() == nml::Material::FRONT ? 1 : -1;
                        if (sign * cglib::dot_product(intersection.normal, rayTransformed.dir) < 0) {
                            continue;
                        }
                    }

                    if (_transformEnabled) {
                        cglib::mat4x4<double> transformMatrix = cglib::mat4x4<double>::convert(_transformMatrix);
                        cglib::mat4x4<double> invTransTransformMatrix = cglib::mat4x4<double>::convert(_invTransTransformMatrix);
                        intersection.pos = cglib::transform_point(intersection.pos, transformMatrix);
                        intersection.normal = cglib::transform_vector(intersection.pos, invTransTransformMatrix);
                    }
                    
                    intersections.push_back(intersection);
                }
            }
        }
    }
    
    int MeshInstance::getDrawCallCount() const {
        if (!_mesh) {
            return 0;
        }
    
        int count = 0;
        for (const std::shared_ptr<Submesh>& submesh : _mesh->getSubmeshList()) {
            count += submesh->getDrawCallCount();
        }
        return count;
    }
    
} }
