#include "GLMesh.h"
#include "GLSubmesh.h"
#include "Package.h"

namespace carto { namespace nml {

    GLMesh::GLMesh(const Mesh& mesh) :
        _submeshList()
    {
        for (int i = 0; i < mesh.submeshes_size(); i++) {
            Submesh submesh = mesh.submeshes(i);
            auto glSubmesh = std::make_shared<GLSubmesh>(submesh);
            _submeshList.push_back(glSubmesh);
        }
    }
    
    GLMesh::GLMesh(const GLMesh& glMesh, const MeshOp& meshOp) :
        _submeshList()
    {
        for (int i = 0; i < meshOp.submesh_op_lists_size(); i++) {
            const SubmeshOpList& submeshOpList = meshOp.submesh_op_lists(i);
            auto glSubmesh = std::make_shared<GLSubmesh>(glMesh, submeshOpList);
            _submeshList.push_back(glSubmesh);
        }
    }
    
    void GLMesh::create() {
        for (auto it = _submeshList.begin(); it != _submeshList.end(); it++) {
            (*it)->create();
        }
    }
    
    void GLMesh::dispose() {
        for (auto it = _submeshList.begin(); it != _submeshList.end(); it++) {
            (*it)->dispose();
        }
    }
    
    const std::vector<std::shared_ptr<GLSubmesh>>& GLMesh::getSubmeshList() const {
        return _submeshList;
    }
    
    int GLMesh::getTotalGeometrySize() const {
        int size = 0;
        for (auto it = _submeshList.begin(); it != _submeshList.end(); it++) {
            size += (*it)->getTotalGeometrySize();
        }
        return size;
    }
    
} }
