/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_MESH_H_
#define _CARTO_NML_MESH_H_

#include "BaseTypes.h"

#include <memory>
#include <vector>

namespace nml {
    class Mesh;
    class MeshOp;
}

namespace carto { namespace nmlgl {
    class Submesh;
        
    class Mesh {
    public:
        Mesh(const nml::Mesh& mesh);
        Mesh(const Mesh& glMesh, const nml::MeshOp& meshOp);

        void create();
        void dispose();

        const std::vector<std::shared_ptr<Submesh>>& getSubmeshList() const;

        int getTotalGeometrySize() const;

    private:
        std::vector<std::shared_ptr<Submesh>> _submeshList;
    };
} }

#endif
