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
#include <set>

#include <cglib/ray.h>

namespace carto {
    class MapPos;
    class MapVec;
    class Options;
    class Shader;
    class ShaderManager;
    class TextureManager;
    class RayIntersectedElement;
    class MapRenderer;
    class ViewState;
    class NMLModelLODTreeDataSource;
    class NMLModelLODTreeLayer;
    
    namespace nml {
        class GLModel;
        class GLResourceManager;
    }

    class NMLModelLODTreeRenderer {
    public:
        NMLModelLODTreeRenderer(const std::weak_ptr<MapRenderer>& mapRenderer, const std::weak_ptr<Options>& options);
        virtual ~NMLModelLODTreeRenderer();
    
        void addDrawData(const std::shared_ptr<NMLModelLODTreeDrawData>& drawData);
        void refreshDrawData();

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
    
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::weak_ptr<Options> _options;

        std::shared_ptr<nml::GLResourceManager> _glResourceManager;
        std::vector<std::shared_ptr<NMLModelLODTreeDrawData> > _tempDrawDatas;
        std::map<long long, std::shared_ptr<ModelNodeDrawRecord> > _drawRecordMap;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
