/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_MESHINSTANCE_H_
#define _CARTO_NML_MESHINSTANCE_H_

#include "BaseTypes.h"

#include <map>
#include <vector>
#include <memory>
#include <string>

namespace nml {
    class MeshInstance;
}

namespace carto { namespace nmlgl {
    class Material;
    class Mesh;
    class Texture;
    class ShaderManager;
        
    class MeshInstance {
    public:
        MeshInstance(const nml::MeshInstance& meshInstance, const std::map<std::string, std::shared_ptr<Mesh>>& meshMap, const std::map<std::string, std::shared_ptr<Texture>>& textureMap);

        void create(ShaderManager& shaderManager);
        void dispose();

        void replaceMesh(const std::string& meshId, const std::shared_ptr<Mesh>& glMesh);
        void replaceTexture(const std::string& textureId, const std::shared_ptr<Texture>& glTexture);

        void draw(const RenderState& renderState);

        void calculateRayIntersections(const Ray& ray, std::vector<RayIntersection>& intersections) const;

        int getDrawCallCount() const;

    private:
        std::string _meshId;
        std::shared_ptr<Mesh> _mesh;
        std::map<std::string, std::shared_ptr<Material>> _materialMap;
        bool _transformEnabled;
        cglib::mat4x4<float> _transformMatrix;
        cglib::mat4x4<float> _invTransTransformMatrix;
    };
} }

#endif
