/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELLODTREERENDERER_H_
#define _CARTO_NMLMODELLODTREERENDERER_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "datasources/components/NMLModelLODTree.h"
#include "renderers/drawdatas/NMLModelLODTreeDrawData.h"

#include <memory>
#include <mutex>
#include <list>
#include <vector>

#include <cglib/ray.h>

namespace carto {
    class MapPos;
    class MapVec;
    class Options;
    class Shader;
    class ShaderManager;
    class TextureManager;
    class RayIntersectedElement;
    class ViewState;
    class NMLModelLODTreeDataSource;
    class NMLModelLODTreeLayer;
    
    namespace nml {
        class GLModel;
        class GLShaderManager;
    }

    class NMLModelLODTreeRenderer {
    public:
        NMLModelLODTreeRenderer();
        virtual ~NMLModelLODTreeRenderer();
    
        void addDrawData(const std::shared_ptr<NMLModelLODTreeDrawData>& drawData);
        void refreshDrawData();

        void setOptions(const std::weak_ptr<Options>& options);
        
        virtual void offsetLayerHorizontally(double offset);
    
        virtual void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        virtual bool onDrawFrame(float deltaSeconds, const ViewState& viewState);
        virtual void onSurfaceDestroyed();

        virtual void calculateRayIntersectedElements(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    protected:
        struct ModelNodeDrawRecord {
            NMLModelLODTreeDrawData drawData;
            ModelNodeDrawRecord* parent;
            std::vector<ModelNodeDrawRecord*> children;
            bool used;
            bool created;
    
            ModelNodeDrawRecord(const NMLModelLODTreeDrawData& drawData) : drawData(drawData), parent(0), children(), used(false), created(false) { }
        };
    
        std::shared_ptr<nml::GLShaderManager> _glShaderManager;
        std::vector<std::shared_ptr<NMLModelLODTreeDrawData> > _tempDrawDatas;
        std::map<long long, std::shared_ptr<ModelNodeDrawRecord> > _drawRecordMap;
        std::weak_ptr<Options> _options;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
