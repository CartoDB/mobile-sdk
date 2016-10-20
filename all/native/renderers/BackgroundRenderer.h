/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BACKGROUNDRENDERER_H_
#define _CARTO_BACKGROUNDRENDERER_H_

#include "graphics/utils/GLContext.h"

#include <memory>

namespace carto {
    class Bitmap;
    class Options;
    class ShaderManager;
    class TextureManager;
    class Shader;
    class Texture;
    class ViewState;
    
    class BackgroundRenderer {
    public:
        explicit BackgroundRenderer(const Options& options);
        virtual ~BackgroundRenderer();
    
        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        void onDrawFrame(const ViewState& viewState);
        void onSurfaceDestroyed();
    
    protected:
        void drawBackground(const ViewState& viewState);
        void drawSky(const ViewState& viewState);
    
        const static float BACKGROUND_COORDS[];
        const static float BACKGROUND_TEX_COORDS[];
        const static int BACKGROUND_VERTEX_COUNT = 4;
    
        const static float SKY_COORDS[];
        const static float SKY_TEX_COORDS[];
        const static int SKY_VERTEX_COUNT = 16;
        const static float SKY_SCALE_MULTIPLIER;
    
        std::shared_ptr<Bitmap> _backgroundBitmap;
        std::shared_ptr<Texture> _backgroundTex;
        float _backgroundCoords[BACKGROUND_VERTEX_COUNT * 3];
        float _backgroundTexCoords[BACKGROUND_VERTEX_COUNT * 2];
    
        std::shared_ptr<Bitmap> _skyBitmap;
        std::shared_ptr<Texture> _skyTex;
        float _skyCoords[SKY_VERTEX_COUNT * 3];
    
        std::shared_ptr<Shader> _shader;
        GLuint _a_coord;
        GLuint _a_texCoord;
        GLuint _u_tex;
        GLuint _u_mvpMat;

        std::shared_ptr<TextureManager> _textureManager;
    
        const Options& _options;
    };
    
}

#endif
