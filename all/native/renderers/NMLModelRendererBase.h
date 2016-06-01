/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELRENDERERBASE_H_
#define _CARTO_NMLMODELRENDERERBASE_H_

#include "nml/BaseTypes.h"

#include <mutex>
#include <memory>

namespace carto {
    class MapPos;
    class MapVec;
    class Shader;
    class ShaderManager;
    class TextureManager;
    class RayIntersectedElement;
    class ViewState;
    
    class NMLModelRendererBase {
    public:
        virtual ~NMLModelRendererBase();
    
        virtual void offsetLayerHorizontally(double offset) = 0;
    
        virtual void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        virtual bool onDrawFrame(float deltaSeconds, const ViewState& viewState);
        virtual void onSurfaceDestroyed();
    
    protected:
        struct GLContext : nmlgl::GLContext {
            GLContext(const Shader &shader);
            virtual ~GLContext();
    
            const Shader& getShader();
    
            void setModelviewProjectionMatrix(const cglib::mat4x4<float>& mvpMatrix);
            virtual GLuint getUniformLocation(const char *name) const;
            virtual GLuint getAttribLocation(const char *name) const;
            virtual void setLocalModelviewMatrix(const float matrix[]);
    
        private:
            const Shader& _shader;
            cglib::mat4x4<float> _mvpMatrix;
        };
    
        NMLModelRendererBase();
    
        virtual bool drawModels(const ViewState& viewState) = 0;

        std::shared_ptr<Shader> _shader;    
        std::shared_ptr<GLContext> _glContext;
        
        mutable std::mutex _mutex;
    };
    
}

#endif
