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

        static const unsigned char DEFAULT_BITMAP[4];
        static const float QUAD_COORDS[12];
        static const float QUAD_TEX_COORDS[8];

        static const std::string SOLID_VERTEX_SHADER;
        static const std::string SOLID_FRAGMENT_SHADER;

        std::weak_ptr<MapRenderer> _mapRenderer;

        Color _color;
        std::shared_ptr<Bitmap> _bitmap;
        std::shared_ptr<Texture> _bitmapTex;
        float _bitmapScale;
        float _quadTexCoords[8];

        std::shared_ptr<Shader> _shader;
        GLuint _a_coord;
        GLuint _a_texCoord;
        GLuint _u_mvpMat;
        GLuint _u_tex;
        GLuint _u_color;

        mutable std::mutex _mutex;
    };
    
}

#endif
