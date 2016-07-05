/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LINERENDERER_H_
#define _CARTO_LINERENDERER_H_

#include "graphics/utils/GLContext.h"

#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <cglib/ray.h>

namespace carto {
    class Bitmap;
    class Line;
    class LineDrawData;
    class Shader;
    class ShaderManager;
    class TextureManager;
    class VectorElement;
    class RayIntersectedElement;
    class VectorLayer;
    class ViewState;
    class StyleTextureCache;
    
    class LineRenderer {
    public:
        LineRenderer();
        virtual ~LineRenderer();
    
        void offsetLayerHorizontally(double offset);
    
        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        void onDrawFrame(float deltaSeconds, StyleTextureCache& styleCache, const ViewState& viewState);
        void onSurfaceDestroyed();
    
        void addElement(const std::shared_ptr<Line>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<Line>& element);
        void removeElement(const std::shared_ptr<Line>& element);
    
        void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    protected:
        friend class PolygonRenderer;
        friend class GeometryCollectionRenderer;

    private:
        static void BuildAndDrawBuffers(GLuint a_color,
                                        GLuint a_coord,
                                        GLuint a_normal,
                                        GLuint a_texCoord,
                                        std::vector<unsigned char>& colorBuf,
                                        std::vector<float>& coordBuf,
                                        std::vector<float>& normalBuf,
                                        std::vector<float>& texCoordBuf,
                                        std::vector<unsigned short>& indexBuf,
                                        std::vector<const LineDrawData*>& drawDataBuffer,
                                        StyleTextureCache& styleCache,
                                        const ViewState& viewState);

        static bool FindElementRayIntersection(const std::shared_ptr<VectorElement>& element,
                                               const std::shared_ptr<LineDrawData>& drawData,
                                               const std::shared_ptr<VectorLayer>& layer,
                                               const cglib::ray3<double>& ray,
                                               const ViewState& viewState,
                                               std::vector<RayIntersectedElement>& results);

        void bind(const ViewState& viewState);
        void unbind();
        
        bool isEmptyBatch() const;
        void addToBatch(const std::shared_ptr<LineDrawData>& drawData, StyleTextureCache& styleCache, const ViewState& viewState);
        void drawBatch(StyleTextureCache& styleCache, const ViewState& viewState);
    
        std::vector<std::shared_ptr<Line> > _elements;
        std::vector<std::shared_ptr<Line> > _tempElements;
        
        std::vector<std::shared_ptr<LineDrawData> > _drawDataBuffer; // this buffer is used to keep objects alive
        std::vector<const LineDrawData*> _lineDrawDataBuffer;
        const Bitmap* _prevBitmap;
    
        std::vector<unsigned char> _colorBuf;
        std::vector<float> _coordBuf;
        std::vector<float> _normalBuf;
        std::vector<float> _texCoordBuf;
        std::vector<unsigned short> _indexBuf;
    
        std::shared_ptr<Shader> _shader;
        GLuint _a_color;
        GLuint _a_coord;
        GLuint _a_normal;
        GLuint _a_texCoord;
        GLuint _u_gamma;
        GLuint _u_dpToPX;
        GLuint _u_unitToDP;
        GLuint _u_mvpMat;
        GLuint _u_tex;
    
        mutable std::mutex _mutex;
    };
    
}

#endif
