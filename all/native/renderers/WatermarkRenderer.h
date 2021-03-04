/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_WATERMARKRENDERER_H_
#define _CARTO_WATERMARKRENDERER_H_

#include "renderers/utils/GLContext.h"

#include <memory>

#include <cglib/vec.h>
#include <cglib/mat.h>

namespace carto {
    class Bitmap;
    class Options;
    class GLResourceManager;
    class Shader;
    class Texture;
    class ViewState;
    
    class WatermarkRenderer {
    public:
        explicit WatermarkRenderer(const Options& options);
        virtual ~WatermarkRenderer();
    
        void onSurfaceCreated(const std::shared_ptr<GLResourceManager>& resourceManager);
        void onSurfaceChanged(int width, int height);
        void onDrawFrame(const ViewState& viewState);
        void onSurfaceDestroyed();
    
    protected:
        void drawWatermark(const ViewState& viewState);
        
        static const int WATERMARK_WIDTH_DP;

        static const std::string WATERMARK_VERTEX_SHADER;
        static const std::string WATERMARK_FRAGMENT_SHADER;
        
        std::shared_ptr<Bitmap> _watermarkBitmap;
        std::shared_ptr<Texture> _watermarkTex;

        float _watermarkCoords[12];
        float _watermarkTexCoords[8];
        
        cglib::mat4x4<float> _modelviewProjectionMat;
        
        std::shared_ptr<Shader> _shader;
        GLuint _u_tex;
        GLuint _u_mvpMat;
        GLuint _a_coord;
        GLuint _a_texCoord;

        std::shared_ptr<GLResourceManager> _glResourceManager;
    
        const Options& _options;
    };
    
}

#endif
