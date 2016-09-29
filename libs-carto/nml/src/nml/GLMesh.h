/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLMESH_H_
#define _CARTO_NML_GLMESH_H_

#include "GLBase.h"

#include <memory>
#include <vector>

namespace carto { namespace nml {
    class Mesh;
    class MeshOp;
    class GLSubmesh;
        
    class GLMesh final {
    public:
        explicit GLMesh(const Mesh& mesh);
        explicit GLMesh(const GLMesh& glMesh, const MeshOp& meshOp);

        void create();
        void dispose();

        const std::vector<std::shared_ptr<GLSubmesh>>& getSubmeshList() const;

        int getTotalGeometrySize() const;

    private:
        std::vector<std::shared_ptr<GLSubmesh>> _submeshList;
    };
} }

#endif
