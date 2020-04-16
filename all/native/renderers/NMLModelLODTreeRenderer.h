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
#include "renderers/utils/GLResource.h"

#include <memory>
#include <mutex>
#include <list>
#include <vector>
#include <set>

#include <cglib/ray.h>

namespace carto {
    class Options;
    class MapRenderer;
    class GLResourceManager;
    class RayIntersectedElement;
    class ViewState;
    class NMLModelLODTreeDataSource;
    class NMLModelLODTreeLayer;
    class NMLResources;
    
    namespace nml {
        class GLModel;
        class GLResourceManager;
    }

    class NMLModelLODTreeRenderer {
    public:
        NMLModelLODTreeRenderer();
        virtual ~NMLModelLODTreeRenderer();
    
        void setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer);

        void offsetLayerHorizontally(double offset);
    
        bool onDrawFrame(float deltaSeconds, const ViewState& viewState);

        void addDrawData(const std::shared_ptr<NMLModelLODTreeDrawData>& drawData);
        void refreshDrawData();

        void calculateRayIntersectedElements(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    protected:
        struct ModelNodeDrawRecord {
            NMLModelLODTreeDrawData drawData;
            ModelNodeDrawRecord* parent;
            std::vector<ModelNodeDrawRecord*> children;
            bool used;
            bool created;
    
            ModelNodeDrawRecord(const NMLModelLODTreeDrawData& drawData) : drawData(drawData), parent(0), children(), used(false), created(false) { }
        };

        bool initializeRenderer();
    
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::weak_ptr<Options> _options;

        std::shared_ptr<NMLResources> _nmlResources;
        std::vector<std::shared_ptr<NMLModelLODTreeDrawData> > _tempDrawDatas;
        std::map<long long, std::shared_ptr<ModelNodeDrawRecord> > _drawRecordMap;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
