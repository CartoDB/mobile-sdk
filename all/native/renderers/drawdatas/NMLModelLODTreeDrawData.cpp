#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "NMLModelLODTreeDrawData.h"

namespace carto {

    NMLModelLODTreeDrawData::NMLModelLODTreeDrawData(const std::shared_ptr<NMLModelLODTree>& modelLODTree, long long nodeId, const std::vector<long long>& parentIds, const std::shared_ptr<nml::GLModel>& glModel) :
        _modelLODTree(modelLODTree),
        _nodeId(nodeId),
        _parentIds(parentIds),
        _localMat(modelLODTree->getLocalMat()),
        _glModel(glModel),
        _proxyMap(modelLODTree->getProxyMap()),
        _isOffset(false)
    {
    }
    
    std::shared_ptr<NMLModelLODTree> NMLModelLODTreeDrawData::getModelLODTree() const {
        return _modelLODTree;
    }
    
    long long NMLModelLODTreeDrawData::getNodeId() const {
        return _nodeId;
    }
    
    const std::vector<long long>& NMLModelLODTreeDrawData::getParentIds() const {
        return _parentIds;
    }
    
    const cglib::mat4x4<double>& NMLModelLODTreeDrawData::getLocalMat() const {
        return _localMat;
    }
    
    const std::shared_ptr<nml::GLModel>& NMLModelLODTreeDrawData::getGLModel() const {
        return _glModel;
    }
    
    const std::shared_ptr<NMLModelLODTree::ProxyMap> NMLModelLODTreeDrawData::getProxyMap() const {
        return _proxyMap;
    }
    
    bool NMLModelLODTreeDrawData::isOffset() const {
        return _isOffset;
    }
    
    void NMLModelLODTreeDrawData::offsetHorizontally(double offset) {
        _localMat = cglib::translate4_matrix(cglib::vec3<double>(offset, 0, 0)) * _localMat;
        _isOffset = true;
    }
    
}

#endif
