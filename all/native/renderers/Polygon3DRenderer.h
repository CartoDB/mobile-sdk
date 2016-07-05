/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGON3DRENDERER_H_
#define _CARTO_POLYGON3DRENDERER_H_

#include "graphics/utils/GLContext.h"

#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <cglib/ray.h>

namespace carto {
    class Bitmap;
    class Options;
    class Polygon3D;
    class Polygon3DDrawData;
    class Shader;
    class ShaderManager;
    class Texture;
    class TextureManager;
    class RayIntersectedElement;
    class VectorLayer;
    class ViewState;
    class StyleTextureCache;
    
    class Polygon3DRenderer {
    public:
        Polygon3DRenderer();
        virtual ~Polygon3DRenderer();
        
        void setOptions(const std::weak_ptr<Options>& options);
    
        void offsetLayerHorizontally(double offset);
    
        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        void onDrawFrame(float deltaSeconds, const ViewState& viewState);
        void onSurfaceDestroyed();
    
        void addElement(const std::shared_ptr<Polygon3D>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<Polygon3D>& element);
        void removeElement(const std::shared_ptr<Polygon3D>& element);
        
        void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    private:
        static void BuildAndDrawBuffers(GLuint a_color,
                                        GLuint a_coord,
                                        GLuint a_normal,
                                        std::vector<unsigned char>& colorBuf,
                                        std::vector<float>& coordBuf,
                                        std::vector<float>& normalBuf,
                                        std::vector<std::shared_ptr<Polygon3DDrawData> >& drawDataBuffer,
                                        const ViewState& viewState);
        
        static std::shared_ptr<Bitmap> GetPolygon3DBitmap();
        
        static std::shared_ptr<Bitmap> _Polygon3DBitmap;
        
        void drawBatch(const ViewState& viewState);
        
        std::shared_ptr<Texture> _polygon3DTex;
    
        std::vector<std::shared_ptr<Polygon3D> > _elements;
        std::vector<std::shared_ptr<Polygon3D> > _tempElements;
        
        std::vector<std::shared_ptr<Polygon3DDrawData> > _drawDataBuffer;
    
        std::vector<unsigned char> _colorBuf;
        std::vector<float> _coordBuf;
        std::vector<float> _normalBuf;
    
        std::shared_ptr<Shader> _shader;
        GLuint _a_color;
        GLuint _a_coord;
        GLuint _a_normal;
        GLuint _a_texCoord;
        GLuint _u_ambientColor;
        GLuint _u_lightColor;
        GLuint _u_lightDir;
        GLuint _u_mvpMat;
        GLuint _u_tex;
    
        std::weak_ptr<Options> _options;
        
        mutable std::mutex _mutex;
    };
    
}

#endif
