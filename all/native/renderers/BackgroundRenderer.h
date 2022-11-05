/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BACKGROUNDRENDERER_H_
#define _CARTO_BACKGROUNDRENDERER_H_

#include "renderers/utils/GLContext.h"

#include <memory>
#include <vector>

#include <cglib/vec.h>

namespace carto {
    class Bitmap;
    class Options;
    class Layers;
    class GLResourceManager;
    class Shader;
    class Texture;
    class ViewState;
    
    class BackgroundRenderer {
    public:
        explicit BackgroundRenderer(const Options& options, const Layers& layers);
        virtual ~BackgroundRenderer();
    
        void onSurfaceCreated(const std::shared_ptr<GLResourceManager>& resourceManager);
        void onDrawFrame(const ViewState& viewState);
        void onSurfaceDestroyed();
    
    protected:
        void drawBackground(const ViewState& viewState);
        void drawSky(const ViewState& viewState);
        void drawContour(const ViewState& viewState);

        static void BuildPlanarSky(std::vector<cglib::vec3<float> >& coords, std::vector<cglib::vec2<float> >& texCoords, const cglib::vec3<double>& cameraPos, const cglib::vec3<double>& focusPos, const cglib::vec3<double>& upVec, double height0, double height1, float coordScale);
        static void BuildSphereSky(std::vector<cglib::vec3<float> >& coords, std::vector<cglib::vec2<float> >& texCoords, const cglib::vec3<double>& cameraPos, const cglib::vec3<double>& upVec, double height0, double height1, float coordScale, int tesselate);
        static void BuildSphereContour(std::vector<cglib::vec3<float> >& coords, const cglib::vec3<double>& cameraPos, const cglib::vec3<double>& upVec, float coordScale, int tesselate);
        static void BuildSphereSurface(std::vector<cglib::vec3<double> >& coords, std::vector<cglib::vec3<float> >& normals, std::vector<cglib::vec2<float> >& texCoords, std::vector<unsigned short>& indices, int tesselateU, int tesselateV);

        enum { SPHERE_TESSELATION_LEVELS_U = 128, SPHERE_TESSELATION_LEVELS_V = 64, SKY_TESSELATION_LEVELS = 128, CONTOUR_TESSELATION_LEVELS = 128 };
    
        static const float PLANE_COORDS[12];
        static const float PLANE_TEX_COORDS[8];
    
        static const float SKY_SCALE_MULTIPLIER_PLANAR;
        static const float SKY_RELATIVE_HEIGHT_PLANAR[2];
        static const float SKY_HEIGHT_RAMP_PLANAR[2];
        static const float SKY_RELATIVE_HEIGHT_SPHERICAL[2];
        static const float SKY_HEIGHT_RAMP_SPHERICAL[2];

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

        std::vector<cglib::vec3<float> > _skyCoords;
        std::vector<cglib::vec2<float> > _skyTexCoords;

        std::vector<cglib::vec3<float> > _contourCoords;

        std::shared_ptr<Shader> _shader;
        GLuint _a_coord;
        GLuint _a_normal;
        GLuint _a_texCoord;
        GLuint _u_tex;
        GLuint _u_lightDir;
        GLuint _u_mvpMat;

        std::shared_ptr<GLResourceManager> _glResourceManager;
    
        const Options& _options;
        const Layers& _layers;
    };
    
}

#endif
