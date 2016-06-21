#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "NMLModelLODTree.h"
#include "geometry/PointGeometry.h"
#include "graphics/ViewState.h"

#include <cassert>

#include <nml/Package.h>

namespace carto {

    NMLModelLODTree::NMLModelLODTree(long long modelLODTreeId, const MapPos& mapPos, std::shared_ptr<Projection> projection, std::shared_ptr<nml::ModelLODTree> sourceModelLODTree, const ProxyMap& proxyMap, const MeshBindingsMap& meshBindingsMap, const TextureBindingsMap& textureBindingsMap) :
        _modelLODTreeId(modelLODTreeId),
        _mapPos(mapPos),
        _localMat(ViewState::GetLocalMat(mapPos, *projection)),
        _sourceModelLODTree(sourceModelLODTree),
        _proxyMap(std::make_shared<ProxyMap>(proxyMap)),
        _meshBindingsMap(meshBindingsMap),
        _textureBindingsMap(textureBindingsMap)
    {
        _nodeParentIds.resize(sourceModelLODTree->nodes_size());
        for (int i = 0; i < sourceModelLODTree->nodes_size(); i++) {
            const nml::ModelLODTreeNode & node = sourceModelLODTree->nodes(i);
            assert(node.id() == i);
            for (int j = 0; j < node.children_ids_size(); j++) {
                assert(node.children_ids(j) < static_cast<int>(_nodeParentIds.size()));
                _nodeParentIds[node.children_ids(j)] = node.id();
            }
        }
    }
    
    NMLModelLODTree::~NMLModelLODTree() {
    }
    
    const MapPos &NMLModelLODTree::getMapPos() const {
        return _mapPos;
    }
    
    const cglib::mat4x4<double> &NMLModelLODTree::getLocalMat() const {
        return _localMat;
    }
    
    std::shared_ptr<nml::ModelLODTree> NMLModelLODTree::getSourceModelLODTree() const {
        return _sourceModelLODTree;
    }
    
    std::shared_ptr<NMLModelLODTree::ProxyMap> NMLModelLODTree::getProxyMap() const {
        return _proxyMap;
    }
    
    const NMLModelLODTree::MeshBindingsMap &NMLModelLODTree::getMeshBindingsMap() const {
        return _meshBindingsMap;
    }
    
    const NMLModelLODTree::TextureBindingsMap &NMLModelLODTree::getTextureBindingsMap() const {
        return _textureBindingsMap;
    }
    
    int NMLModelLODTree::getSourceNodeCount() const {
        return _sourceModelLODTree->nodes_size();
    }
    
    const nml::ModelLODTreeNode *NMLModelLODTree::getSourceNode(int nodeId) const {
        assert(nodeId >= 0 && nodeId < _sourceModelLODTree->nodes_size());
        return &_sourceModelLODTree->nodes(nodeId);
    }
    
    int NMLModelLODTree::getNodeParentId(int nodeId) const {
        assert(nodeId >= 0 && nodeId < (int) _nodeParentIds.size());
        return _nodeParentIds[nodeId];
    }
    
    long long NMLModelLODTree::getGlobalNodeId(int nodeId) const {
        return (_modelLODTreeId << 24) + nodeId;
    }
    
}

#endif
