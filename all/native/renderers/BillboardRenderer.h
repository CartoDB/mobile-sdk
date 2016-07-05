/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BILLBOARDRENDERER_H_
#define _CARTO_BILLBOARDRENDERER_H_

#include "core/MapPos.h"
#include "graphics/utils/GLContext.h"

#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <cglib/vec.h>
#include <cglib/ray.h>

namespace carto {
    class Billboard;
    class BillboardDrawData;
    class BillboardSorter;
    class Bitmap;
    class Shader;
    class ShaderManager;
    class TextureManager;
    class RayIntersectedElement;
    class VectorLayer;
    class ViewState;
    class StyleTextureCache;
    
    class BillboardRenderer {
    public:
        static void CalculateBillboardCoords(const BillboardDrawData& drawData, const ViewState& viewState,
                                             std::vector<float>& coordBuf, int drawDataIndex);
        
        BillboardRenderer();
        virtual ~BillboardRenderer();
    
        virtual void offsetLayerHorizontally(double offset);
    
        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        void onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState);
        void onDrawFrameSorted(float deltaSeconds, const std::vector<std::shared_ptr<BillboardDrawData> >& billboardDrawDatas, StyleTextureCache& styleCache, const ViewState& viewState);
        void onSurfaceDestroyed();
    
        std::size_t getElementCount() const;
        void addElement(const std::shared_ptr<Billboard>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<Billboard>& element);
        void removeElement(const std::shared_ptr<Billboard>& element);
        
        void setLayer(const std::shared_ptr<VectorLayer>& layer);
        std::shared_ptr<VectorLayer> getLayer() const;
    
        virtual void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    private:
        static void BuildAndDrawBuffers(GLuint a_color,
                                        GLuint a_coord,
                                        GLuint a_texCoord,
                                        std::vector<unsigned char>& colorBuf,
                                        std::vector<float>& coordBuf,
                                        std::vector<unsigned short>& indexBuf,
                                        std::vector<float>& texCoordBuf,
                                        std::vector<std::shared_ptr<BillboardDrawData> >& drawDataBuffer,
                                        const cglib::vec2<float>& texCoordScale,
                                        StyleTextureCache& styleCache,
                                        const ViewState& viewState);
        
        bool calculateBaseBillboardDrawData(const std::shared_ptr<BillboardDrawData>& drawData, const ViewState& viewState);
        
        void drawBatch(StyleTextureCache& styleCache, const ViewState& viewState);
        
        std::weak_ptr<VectorLayer> _layer;
    
        std::vector<std::shared_ptr<Billboard> > _elements;
        std::vector<std::shared_ptr<Billboard> > _tempElements;
        
        std::vector<std::shared_ptr<BillboardDrawData> > _drawDataBuffer;
        
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
    
        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
