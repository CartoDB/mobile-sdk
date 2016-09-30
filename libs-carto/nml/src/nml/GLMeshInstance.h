/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLMESHINSTANCE_H_
#define _CARTO_NML_GLMESHINSTANCE_H_

#include "GLBase.h"

#include <map>
#include <vector>
#include <memory>
#include <string>

namespace carto { namespace nml {
    class MeshInstance;
    class GLMaterial;
    class GLMesh;
    class GLTexture;
    class GLShaderManager;
        
    class GLMeshInstance final {
    public:
        explicit GLMeshInstance(const MeshInstance& meshInstance, const std::map<std::string, std::shared_ptr<GLMesh>>& meshMap, const std::map<std::string, std::shared_ptr<GLTexture>>& textureMap);

        void create(GLShaderManager& shaderManager);
        void dispose();

        void replaceMesh(const std::string& meshId, const std::shared_ptr<GLMesh>& mesh);
        void replaceTexture(const std::string& textureId, const std::shared_ptr<GLTexture>& texture);

        void draw(const RenderState& renderState);

        void calculateRayIntersections(const cglib::ray3<double>& ray, std::vector<RayIntersection>& intersections) const;

        int getDrawCallCount() const;

    private:
        std::string _meshId;
        std::shared_ptr<GLMesh> _mesh;
        std::map<std::string, std::shared_ptr<GLMaterial>> _materialMap;
        bool _transformEnabled;
        cglib::mat4x4<float> _transformMatrix;
        cglib::mat4x4<float> _invTransTransformMatrix;
    };
} }

#endif
