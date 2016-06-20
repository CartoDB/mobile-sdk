/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELLODTREERENDERER_H_
#define _CARTO_NMLMODELLODTREERENDERER_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "datasources/components/NMLModelLODTree.h"
#include "renderers/NMLModelRendererBase.h"
#include "renderers/drawdatas/NMLModelLODTreeDrawData.h"
#include "nml/Model.h"

#include <list>
#include <vector>

namespace carto {
    class NMLModelLODTreeDataSource;
    class NMLModelLODTreeLayer;
    
    class NMLModelLODTreeRenderer : public NMLModelRendererBase {
    public:
        NMLModelLODTreeRenderer();
        virtual ~NMLModelLODTreeRenderer();
    
        void addDrawData(const std::shared_ptr<NMLModelLODTreeDrawData>& drawData);
        void refreshDrawData();
        
        virtual void offsetLayerHorizontally(double offset);
    
        virtual void calculateRayIntersectedElements(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const MapPos& rayOrig, const MapVec& rayDir, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    protected:
        struct ModelNodeDrawRecord {
            NMLModelLODTreeDrawData drawData;
            ModelNodeDrawRecord* parent;
            std::vector<ModelNodeDrawRecord *> children;
            bool used;
            bool created;
    
            ModelNodeDrawRecord(const NMLModelLODTreeDrawData& drawData) : drawData(drawData), parent(0), children(), used(false), created(false) { }
        };
    
        std::vector<std::shared_ptr<NMLModelLODTreeDrawData> > _tempDrawDatas;
        std::map<long long, std::shared_ptr<ModelNodeDrawRecord> > _drawRecordMap;
    
        virtual bool drawModels(const ViewState& viewState);
    };
    
}

#endif

#endif
