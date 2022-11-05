/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SOLIDRENDERER_H_
#define _CARTO_SOLIDRENDERER_H_

#include "graphics/Color.h"
#include "renderers/utils/GLContext.h"

#include <memory>
#include <mutex>
#include <vector>

#include <cglib/vec.h>

namespace carto {
    class Bitmap;
    class Options;
    class MapRenderer;
    class Shader;
    class Texture;
    class ViewState;

    class SolidRenderer {
    public:
        SolidRenderer();
        virtual ~SolidRenderer();
    
        void setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer);

        void onDrawFrame(const ViewState& viewState);

        void setColor(const Color& color);
        void setBitmap(const std::shared_ptr<Bitmap>& bitmap, float scale);

    private:
        bool initializeRenderer();
        void drawBackground(const ViewState& viewState);

        static void BuildSphereSurface(std::vector<cglib::vec3<double> >& coords, std::vector<cglib::vec3<float> >& normals, std::vector<cglib::vec2<float> >& texCoords, std::vector<unsigned short>& indices, const cglib::vec3<double>& cameraPos, int tesselateU, int tesselateV);
    
        enum { SPHERE_TESSELATION_LEVELS_U = 128, SPHERE_TESSELATION_LEVELS_V = 64, SKY_TESSELATION_LEVELS = 128, CONTOUR_TESSELATION_LEVELS = 128 };

        static const unsigned char DEFAULT_BITMAP[4];

        static const float PLANE_COORDS[12];
        static const float PLANE_TEX_COORDS[8];
    
        static const std::string SOLID_VERTEX_SHADER;
        static const std::string SOLID_FRAGMENT_SHADER;

        std::shared_ptr<Bitmap> _backgroundBitmap;
        std::shared_ptr<Texture> _backgroundTex;
        std::vector<float> _backgroundVertices;
    
        std::vector<cglib::vec3<double> > _backgroundCoords;
        std::vector<cglib::vec3<float> > _backgroundNormals;
        std::vector<cglib::vec2<float> > _backgroundTexCoords;
        std::vector<unsigned short> _backgroundIndices;

        std::weak_ptr<Options> _options;
        std::weak_ptr<MapRenderer> _mapRenderer;

        Color _color;
        std::shared_ptr<Bitmap> _bitmap;
        std::shared_ptr<Texture> _bitmapTex;
        float _bitmapScale;

        std::shared_ptr<Shader> _shader;
        GLuint _a_coord;
        GLuint _a_normal;
        GLuint _a_texCoord;
        GLuint _u_mvpMat;
        GLuint _u_tex;
        GLuint _u_lightDir;
        GLuint _u_color;

        mutable std::mutex _mutex;
    };
    
}

#endif
