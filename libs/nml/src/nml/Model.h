/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_MODEL_H_
#define _CARTO_NML_MODEL_H_

#include "BaseTypes.h"

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

namespace nml {
    class Model;
    class MeshOp;
}

namespace carto { namespace nmlgl {
    class Mesh;
    class MeshInstance;
    class Texture;

    class Model {
    public:
        Model(const nml::Model& model);
            
        void create(const std::shared_ptr<GLContext>& gl);
        void dispose(const std::shared_ptr<GLContext>& gl);

        void replaceMesh(const std::string& id, const std::shared_ptr<Mesh>& glMesh);
        void replaceMesh(const std::string& id, const std::shared_ptr<Mesh>& glMesh, const std::shared_ptr<nml::MeshOp>& meshOp);
        void replaceTexture(const std::string& id, const std::shared_ptr<Texture>& glTexture);

        void draw(const std::shared_ptr<GLContext>& gl);

        void calculateRayIntersections(const Ray& ray, std::vector<RayIntersection>& intersections) const;

        cglib::bbox3<float> getBounds() const;

        int getDrawCallCount() const;
        int getTotalGeometrySize() const;

    private:
        typedef std::pair<std::shared_ptr<Mesh>, std::shared_ptr<nml::MeshOp> > MeshMeshOpPair;

        cglib::bbox3<float> _bounds = cglib::bbox3<float>::smallest();
        std::map<std::string, MeshMeshOpPair> _meshMap;
        std::map<std::string, std::shared_ptr<Texture>> _textureMap;
        std::vector<std::shared_ptr<MeshInstance>> _meshInstanceList;
        mutable std::mutex _mutex;
    };
} }

#endif
