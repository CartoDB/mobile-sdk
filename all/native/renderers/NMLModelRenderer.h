/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELRENDERER_H_
#define _CARTO_NMLMODELRENDERER_H_

#include "NMLModelRendererBase.h"
#include "renderers/drawdatas/NMLModelDrawData.h"
#include "vectorelements/NMLModel.h"

#include <list>
#include <vector>

namespace carto {
    class VectorLayer;
    
    namespace nmlgl {
        class Model;
    }
    
    class NMLModelRenderer : public NMLModelRendererBase {
    public:
        NMLModelRenderer();
        virtual ~NMLModelRenderer();
        
        void addElement(const std::shared_ptr<NMLModel>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<NMLModel>& element);
        void removeElement(const std::shared_ptr<NMLModel>& element);
    
        virtual void offsetLayerHorizontally(double offset);
        
        virtual void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const MapPos& rayOrig, const MapVec& rayDir, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    protected:
        virtual bool drawModels(const ViewState& viewState);
    
    private:
        typedef std::vector<std::shared_ptr<NMLModel> > ElementsVector;
        typedef std::map<std::shared_ptr<nml::Model>, std::shared_ptr<nmlgl::Model>> GLModelMap;
    
        GLModelMap _glModelMap;
        ElementsVector _elements;
        ElementsVector _tempElements;
    };
    
}

#endif
