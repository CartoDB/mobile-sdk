/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELLODTREEDRAWDATA_H_
#define _CARTO_NMLMODELLODTREEDRAWDATA_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "datasources/components/NMLModelLODTree.h"

#include <memory>

namespace carto {
    namespace nmlgl {
        class Model;
    }
    
    class NMLModelLODTreeDrawData {
    public:
        NMLModelLODTreeDrawData(const std::shared_ptr<NMLModelLODTree>& modelLODTree, long long nodeId, const std::vector<long long>& parentIds, const std::shared_ptr<nmlgl::Model>& glModel);
    
        std::shared_ptr<NMLModelLODTree> getModelLODTree() const;
        long long getNodeId() const;
        const std::vector<long long>& getParentIds() const;
        const cglib::mat4x4<double>& getLocalMat() const;
        const std::shared_ptr<nmlgl::Model>& getGLModel() const;
        const std::shared_ptr<NMLModelLODTree::ProxyMap> getProxyMap() const;
        
        virtual bool isOffset() const;
        virtual void offsetHorizontally(double offset);
    
    private:
        std::shared_ptr<NMLModelLODTree> _modelLODTree;
        long long _nodeId;
        std::vector<long long> _parentIds;
        cglib::mat4x4<double> _localMat;
        std::shared_ptr<nmlgl::Model> _glModel;
        std::shared_ptr<NMLModelLODTree::ProxyMap> _proxyMap;
        bool _isOffset;
    };
    
}

#endif

#endif
