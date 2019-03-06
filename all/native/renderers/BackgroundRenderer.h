/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BACKGROUNDRENDERER_H_
#define _CARTO_BACKGROUNDRENDERER_H_

#include "graphics/utils/GLContext.h"

#include <memory>
#include <vector>

#include <cglib/vec.h>

namespace carto {
    class Bitmap;
    class Options;
    class Layers;
    class ShaderManager;
    class TextureManager;
    class Shader;
    class Texture;
    class ViewState;
    
    class BackgroundRenderer {
    public:
        explicit BackgroundRenderer(const Options& options, const Layers& layers);
        virtual ~BackgroundRenderer();
    
        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        void onDrawFrame(const ViewState& viewState);
        void onSurfaceDestroyed();
    
    protected:
        static void BuildSphereSky(std::vector<cglib::vec3<double> >& coords, std::vector<cglib::vec2<float> >& texCoords, std::vector<unsigned short>& indices, const cglib::vec3<double>& cameraPos, const cglib::vec3<double>& upVec, double height0, double height1, int tesselate);
        static void BuildSphereSurface(std::vector<cglib::vec3<double> >& coords, std::vector<cglib::vec3<float> >& normals, std::vector<cglib::vec2<float> >& texCoords, std::vector<unsigned short>& indices, int tesselateU, int tesselateV);

        void drawBackground(const ViewState& viewState);
        void drawSky(const ViewState& viewState);

        enum { SURFACE_TESSELATION_LEVELS = 64, SKY_TESSELATION_LEVELS = 128 };
    
        static const float BACKGROUND_COORDS[12];
        static const float BACKGROUND_TEX_COORDS[8];
    
        static const float SKY_COORDS[48];
        static const float SKY_TEX_COORDS[48];
        static const float SKY_SCALE_MULTIPLIER;
        static const float SKY_RELATIVE_HEIGHT;

        static const std::string BACKGROUND_VERTEX_SHADER;
        static const std::string BACKGROUND_FRAGMENT_SHADER;

        std::shared_ptr<Bitmap> _backgroundBitmap;
        std::shared_ptr<Texture> _backgroundTex;
        std::vector<float> _backgroundVertices;
    
        std::shared_ptr<Bitmap> _skyBitmap;
        std::shared_ptr<Texture> _skyTex;
        std::vector<float> _skyVertices;
    
        std::vector<cglib::vec3<double> > _backgroundCoords;
        std::vector<cglib::vec3<float> > _backgroundNormals;
        std::vector<cglib::vec2<float> > _backgroundTexCoords;
        std::vector<unsigned short> _backgroundIndices;

        std::vector<cglib::vec3<double> > _skyCoords;
        std::vector<cglib::vec2<float> > _skyTexCoords;
        std::vector<unsigned short> _skyIndices;

        std::shared_ptr<Shader> _shader;
        GLuint _a_coord;
        GLuint _a_normal;
        GLuint _a_texCoord;
        GLuint _u_tex;
        GLuint _u_lightDir;
        GLuint _u_mvpMat;

        std::shared_ptr<TextureManager> _textureManager;
    
        const Options& _options;
        const Layers& _layers;
    };
    
}

#endif
