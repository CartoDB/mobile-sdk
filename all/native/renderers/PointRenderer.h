/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POINTRENDERER_H_
#define _CARTO_POINTRENDERER_H_

#include "graphics/utils/GLContext.h"

#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <cglib/vec.h>
#include <cglib/ray.h>

namespace carto {
    class Bitmap;
    class Point;
    class PointDrawData;
    class Shader;
    class ShaderManager;
    class TextureManager;
    class VectorElement;
    class RayIntersectedElement;
    class VectorLayer;
    class ViewState;
    class StyleTextureCache;

    class PointRenderer {
    public:
        PointRenderer();
        virtual ~PointRenderer();
    
        virtual void offsetLayerHorizontally(double offset);
    
        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        void onDrawFrame(float deltaSeconds, StyleTextureCache& styleCache, const ViewState& viewState);
        void onSurfaceDestroyed();
    
        void addElement(const std::shared_ptr<Point>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<Point>& element);
        void removeElement(const std::shared_ptr<Point>& element);
    
        virtual void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;

    protected:
        friend class GeometryCollectionRenderer;

    private:
        static void BuildAndDrawBuffers(GLuint a_color,
                                        GLuint a_coord,
                                        GLuint a_texCoord,
                                        std::vector<unsigned char>& colorBuf,
                                        std::vector<float>& coordBuf,
                                        std::vector<unsigned short>& indexBuf,
                                        std::vector<float>& texCoordBuf,
                                        std::vector<std::shared_ptr<PointDrawData> >& drawDataBuffer,
                                        const cglib::vec2<float>& texCoordScale,
                                        StyleTextureCache& styleCache,
                                        const ViewState& viewState);
        
        static bool FindElementRayIntersection(const std::shared_ptr<VectorElement>& element,
                                               const std::shared_ptr<PointDrawData>& drawData,
                                               const std::shared_ptr<VectorLayer>& layer,
                                               const cglib::ray3<double>& ray,
                                               const ViewState& viewState,
                                               std::vector<RayIntersectedElement>& results);

        void bind(const ViewState& viewState);
        void unbind();
        
        bool isEmptyBatch() const;
        void addToBatch(const std::shared_ptr<PointDrawData>& drawData, StyleTextureCache& styleCache, const ViewState& viewState);
        void drawBatch(StyleTextureCache& styleCache, const ViewState& viewState);
        void drawBuffers(int indexCount) const;
    
        std::vector<std::shared_ptr<Point> > _elements;
        std::vector<std::shared_ptr<Point> > _tempElements;
        
        std::vector<std::shared_ptr<PointDrawData> > _drawDataBuffer;
        const Bitmap* _prevBitmap;
    
        std::vector<unsigned char> _colorBuf;
        std::vector<float> _coordBuf;
        std::vector<unsigned short> _indexBuf;
        std::vector<float> _texCoordBuf;
    
        std::shared_ptr<Shader> _shader;
        GLuint _a_color;
        GLuint _a_coord;
        GLuint _a_texCoord;
        GLuint _u_mvpMat;
        GLuint _u_tex;
    
        mutable std::mutex _mutex;
    };
    
}

#endif
