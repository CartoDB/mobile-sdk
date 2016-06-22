/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLMODEL_H_
#define _CARTO_NML_GLMODEL_H_

#include "GLBase.h"

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

namespace carto { namespace nml {
    class Model;
    class MeshOp;
    class GLMesh;
    class GLMeshInstance;
    class GLTexture;
    class GLShaderManager;

    class GLModel {
    public:
        GLModel(const Model& model);
            
        void create(GLShaderManager& shaderManager);
        void dispose();

        void replaceMesh(const std::string& id, const std::shared_ptr<GLMesh>& mesh);
        void replaceMesh(const std::string& id, const std::shared_ptr<GLMesh>& mesh, const std::shared_ptr<MeshOp>& meshOp);
        void replaceTexture(const std::string& id, const std::shared_ptr<GLTexture>& texture);

        void draw(const RenderState& renderState);

        void calculateRayIntersections(const cglib::ray3<double>& ray, std::vector<RayIntersection>& intersections) const;

        cglib::bbox3<float> getBounds() const;

        int getDrawCallCount() const;
        int getTotalGeometrySize() const;

    private:
        typedef std::pair<std::shared_ptr<GLMesh>, std::shared_ptr<MeshOp>> GLMeshMeshOpPair;

        cglib::bbox3<float> _bounds = cglib::bbox3<float>::smallest();
        std::map<std::string, GLMeshMeshOpPair> _meshMap;
        std::map<std::string, std::shared_ptr<GLTexture>> _textureMap;
        std::vector<std::shared_ptr<GLMeshInstance>> _meshInstanceList;
        mutable std::mutex _mutex;
    };
} }

#endif
