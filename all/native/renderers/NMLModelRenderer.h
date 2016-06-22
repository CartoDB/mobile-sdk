/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELRENDERER_H_
#define _CARTO_NMLMODELRENDERER_H_

#include "renderers/drawdatas/NMLModelDrawData.h"
#include "vectorelements/NMLModel.h"

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
    class VectorLayer;
    
    namespace nml {
        class GLModel;
        class GLShaderManager;
    }
    
    class NMLModelRenderer {
    public:
        NMLModelRenderer();
        virtual ~NMLModelRenderer();
        
        void addElement(const std::shared_ptr<NMLModel>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<NMLModel>& element);
        void removeElement(const std::shared_ptr<NMLModel>& element);

        void setOptions(const std::weak_ptr<Options>& options);
    
        virtual void offsetLayerHorizontally(double offset);
        
        virtual void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        virtual bool onDrawFrame(float deltaSeconds, const ViewState& viewState);
        virtual void onSurfaceDestroyed();

        virtual void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    private:
        std::shared_ptr<nml::GLShaderManager> _glShaderManager;
        std::map<std::shared_ptr<nml::Model>, std::shared_ptr<nml::GLModel> > _glModelMap;
        std::vector<std::shared_ptr<NMLModel> > _elements;
        std::vector<std::shared_ptr<NMLModel> > _tempElements;

        std::weak_ptr<Options> _options;

        mutable std::mutex _mutex;
    };
    
}

#endif
