/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGONRENDERER_H_
#define _CARTO_POLYGONRENDERER_H_

#include "graphics/utils/GLContext.h"
#include "renderers/LineRenderer.h"

#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <cglib/ray.h>

namespace carto {
    class Bitmap;
    class LineDrawData;
    class Polygon;
    class PolygonDrawData;
    class Shader;
    class ShaderManager;
    class TextureManager;
    class VectorElement;
    class RayIntersectedElement;
    class VectorLayer;
    class ViewState;
    class StyleTextureCache;
    
    class PolygonRenderer {
    public:
        PolygonRenderer();
        virtual ~PolygonRenderer();
    
        void offsetLayerHorizontally(double offset);
    
        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        void onDrawFrame(float deltaSeconds, StyleTextureCache& styleCache, const ViewState& viewState);
        void onSurfaceDestroyed();
    
        void addElement(const std::shared_ptr<Polygon>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<Polygon>& element);
        void removeElement(const std::shared_ptr<Polygon>& element);
        
        void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    protected:
        friend class GeometryCollectionRenderer;

    private:
        static void BuildAndDrawBuffers(GLuint a_color,
                                        GLuint a_coord,
                                        std::vector<unsigned char>& colorBuf,
                                        std::vector<float>& coordBuf,
                                        std::vector<unsigned short>& indexBuf,
                                        std::vector<std::shared_ptr<PolygonDrawData> >& drawDataBuffer,
                                        StyleTextureCache& styleCache,
                                        const ViewState& viewState);
        
        static bool FindElementRayIntersection(const std::shared_ptr<VectorElement>& element,
                                               const std::shared_ptr<PolygonDrawData>& drawData,
                                               const std::shared_ptr<VectorLayer>& layer,
                                               const cglib::ray3<double>& ray,
                                               const ViewState& viewState,
                                               std::vector<RayIntersectedElement>& results);

        void bind(const ViewState& viewState);
        void unbind();
        
        bool isEmptyBatch() const;
        void addToBatch(const std::shared_ptr<PolygonDrawData>& drawData, StyleTextureCache& styleCache, const ViewState& viewState);
        void drawBatch(StyleTextureCache& styleCache, const ViewState& viewState);
    
        std::vector<std::shared_ptr<Polygon> > _elements;
        std::vector<std::shared_ptr<Polygon> > _tempElements;
        
        std::vector<std::shared_ptr<PolygonDrawData> > _drawDataBuffer;
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

        LineRenderer _lineRenderer;
    
        mutable std::mutex _mutex;
    };
    
}

#endif
