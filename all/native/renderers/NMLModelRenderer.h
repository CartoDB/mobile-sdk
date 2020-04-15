/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELRENDERER_H_
#define _CARTO_NMLMODELRENDERER_H_

#include "renderers/utils/GLResource.h"

#include <memory>
#include <mutex>
#include <list>
#include <vector>
#include <map>

#include <cglib/ray.h>

namespace carto {
    class Options;
    class MapRenderer;
    class GLResourceManager;
    class RayIntersectedElement;
    class NMLModel;
    class ViewState;
    class VectorLayer;
    
    namespace nml {
        class Model;
        class GLModel;
        class GLResourceManager;
    }
    
    class NMLModelRenderer {
    public:
        NMLModelRenderer();
        virtual ~NMLModelRenderer();
        
        void setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer);
    
        void offsetLayerHorizontally(double offset);
        
        bool onDrawFrame(float deltaSeconds, const ViewState& viewState);

        void addElement(const std::shared_ptr<NMLModel>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<NMLModel>& element);
        void removeElement(const std::shared_ptr<NMLModel>& element);

        void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    private:
        class GLBaseResourceManager : public GLResource {
        public:
            virtual ~GLBaseResourceManager();

            std::shared_ptr<nml::GLResourceManager> get() const {
                std::lock_guard<std::mutex> lock(_mutex);
                return _resourceManager;
            }

            nml::GLResourceManager* operator -> () const {
                std::lock_guard<std::mutex> lock(_mutex);
                return _resourceManager.get();
            }

        protected:
            friend GLResourceManager;

            GLBaseResourceManager(const std::shared_ptr<GLResourceManager>& manager);

            virtual void create() const;
            virtual void destroy() const;

        private:
            mutable std::shared_ptr<nml::GLResourceManager> _resourceManager;

            mutable std::mutex _mutex;
        };

        bool initializeRenderer();

        std::weak_ptr<MapRenderer> _mapRenderer;
        std::weak_ptr<Options> _options;

        std::shared_ptr<GLBaseResourceManager> _glBaseResourceManager;
        std::map<std::weak_ptr<nml::Model>, std::shared_ptr<nml::GLModel>, std::owner_less<std::weak_ptr<nml::Model> > > _glModelMap;
        std::vector<std::shared_ptr<NMLModel> > _elements;
        std::vector<std::shared_ptr<NMLModel> > _tempElements;

        mutable std::mutex _mutex;
    };
    
}

#endif
