/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELLODTREE_H_
#define _CARTO_NMLMODELLODTREE_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "core/MapPos.h"
#include "projections/Projection.h"

#include <memory>
#include <map>
#include <list>
#include <vector>

#include <cglib/mat.h>

namespace carto {
    namespace nml {
        class Mesh;
        class MeshOp;
        class Texture;
        class ModelLODTree;
        class ModelLODTreeNode;
    }

    class NMLModelLODTree : public std::enable_shared_from_this<NMLModelLODTree> {
    public:
        struct Proxy {
            long long id;
            MapPos mapPos;
            std::map<std::string, std::string> metaData;

            Proxy(long long id, const MapPos& mapPos, const std::map<std::string, std::string>& metaData) : id(id), mapPos(mapPos), metaData(metaData) { }
        };
    
        struct MeshBinding {
            long long meshId;
            std::string localId;
            std::shared_ptr<nml::MeshOp> meshOp;
    
            MeshBinding(long long meshId, const std::string& localId) : meshId(meshId), localId(localId), meshOp() { }
            MeshBinding(long long meshId, const std::string& localId, std::shared_ptr<nml::MeshOp> meshOp) : meshId(meshId), localId(localId), meshOp(meshOp) { }
        };
    
        struct TextureBinding {
            long long textureId;
            int level;
            std::string localId;
    
            TextureBinding(long long textureId, int level, const std::string& localId) : textureId(textureId), level(level), localId(localId) { }
        };

        typedef nml::Mesh Mesh;
        typedef nml::MeshOp MeshOp;
        typedef nml::Texture Texture;
        typedef std::list<MeshBinding> MeshBindingList;
        typedef std::list<TextureBinding> TextureBindingList;
        typedef std::map<int, MeshBindingList> MeshBindingsMap;
        typedef std::map<int, TextureBindingList> TextureBindingsMap;
        typedef std::map<int, Proxy> ProxyMap;
        
        NMLModelLODTree(long long modelLODTreeId, const MapPos& mapPos, std::shared_ptr<Projection> projection, std::shared_ptr<nml::ModelLODTree> sourceModelLODTree, const ProxyMap& proxyMap, const MeshBindingsMap& meshBindingsMap, const TextureBindingsMap& textureBindingsMap);
        virtual ~NMLModelLODTree();
    
        const MapPos& getMapPos() const;
        const cglib::mat4x4<double>& getLocalMat() const;
        std::shared_ptr<nml::ModelLODTree> getSourceModelLODTree() const;
        std::shared_ptr<ProxyMap> getProxyMap() const;
    
        const MeshBindingsMap& getMeshBindingsMap() const;
        const TextureBindingsMap& getTextureBindingsMap() const;
    
        int getSourceNodeCount() const;
        const nml::ModelLODTreeNode* getSourceNode(int nodeId) const;
        int getNodeParentId(int nodeId) const;
        long long getGlobalNodeId(int nodeId) const;
    
    private:
        long long _modelLODTreeId;
        MapPos _mapPos;
        cglib::mat4x4<double> _localMat;
        std::shared_ptr<nml::ModelLODTree> _sourceModelLODTree;
        std::shared_ptr<ProxyMap> _proxyMap;
        MeshBindingsMap _meshBindingsMap;
        TextureBindingsMap _textureBindingsMap;
        std::vector<int> _nodeParentIds;
    };
    
}

#endif

#endif
