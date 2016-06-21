#include "Mesh.h"
#include "Submesh.h"

#include "nmlpackage/NMLPackage.pb.h"

namespace carto { namespace nmlgl {

    Mesh::Mesh(const nml::Mesh& mesh) :
        _submeshList()
    {
        for (int i = 0; i < mesh.submeshes_size(); i++) {
            nml::Submesh submesh = mesh.submeshes(i);
            auto glSubmesh = std::make_shared<Submesh>(submesh);
            _submeshList.push_back(glSubmesh);
        }
    }
    
    Mesh::Mesh(const Mesh& Mesh, const nml::MeshOp& meshOp) :
        _submeshList()
    {
        for (int i = 0; i < meshOp.submesh_op_lists_size(); i++) {
            const nml::SubmeshOpList &submeshOpList = meshOp.submesh_op_lists(i);
            auto glSubmesh = std::make_shared<Submesh>(Mesh, submeshOpList);
            _submeshList.push_back(glSubmesh);
        }
    }
    
    void Mesh::create() {
        for (auto it = _submeshList.begin(); it != _submeshList.end(); it++) {
            (*it)->create();
        }
    }
    
    void Mesh::dispose() {
        for (auto it = _submeshList.begin(); it != _submeshList.end(); it++) {
            (*it)->dispose();
        }
    }
    
    const std::vector<std::shared_ptr<Submesh>>& Mesh::getSubmeshList() const {
        return _submeshList;
    }
    
    int Mesh::getTotalGeometrySize() const {
        int size = 0;
        for (auto it = _submeshList.begin(); it != _submeshList.end(); it++) {
            size += (*it)->getTotalGeometrySize();
        }
        return size;
    }
    
} }
