/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BILLBOARDRENDERER_H_
#define _CARTO_BILLBOARDRENDERER_H_

#include "core/MapPos.h"
#include "renderers/utils/GLContext.h"
#include "renderers/utils/BitmapTextureCache.h"

#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <map>

#include <cglib/vec.h>
#include <cglib/mat.h>
#include <cglib/ray.h>

namespace carto {
    class Billboard;
    class BillboardDrawData;
    class BillboardSorter;
    class Bitmap;
    class Options;
    class MapRenderer;
    class Shader;
    class NMLModel;
    class NMLModelDrawData;
    class NMLResources;
    class RayIntersectedElement;
    class VectorLayer;
    class ViewState;
    
    namespace nml {
        class Model;
        class GLModel;
    }
    
    class BillboardRenderer : public std::enable_shared_from_this<BillboardRenderer> {
    public:
        BillboardRenderer();
        virtual ~BillboardRenderer();
    
        void setComponents(const std::weak_ptr<VectorLayer>& layer, const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer);

        void offsetLayerHorizontally(double offset);
    
        bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState);
        void onDrawFrameSorted(float deltaSeconds, const std::vector<std::shared_ptr<BillboardDrawData> >& billboardDrawDatas, const ViewState& viewState);
    
        std::size_t getElementCount() const;
        void addElement(const std::shared_ptr<Billboard>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<Billboard>& element);
        void removeElement(const std::shared_ptr<Billboard>& element);

        void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;

        static bool UpdateBillboardAnimationState(BillboardDrawData& drawData, float deltaSeconds);
        static void CalculateBillboardAxis(const BillboardDrawData& drawData, const ViewState& viewState, cglib::vec3<float>& xAxis, cglib::vec3<float>& yAxis);
        static bool CalculateBillboardCoords(const BillboardDrawData& drawData, const ViewState& viewState, std::vector<float>& coordBuf, std::size_t drawDataIndex, float sizeScale = 1.0f);
        static bool CalculateBaseBillboardDrawData(BillboardDrawData& drawData, const ViewState& viewState);
        static bool CalculateNMLModelMatrix(const NMLModelDrawData& drawData, const ViewState& viewState, cglib::mat4x4<double>& modelMat, float sizeScale = 1.0f);
        
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
                                        float opacity,
                                        const ViewState& viewState);

        bool initializeRenderer();
        void drawBatch(float opacity, const ViewState& viewState);

        bool initializeNMLRenderer();
        void drawNMLBatch(float opacity, const ViewState& viewState);
        void calculateNMLRayIntersections(const std::shared_ptr<NMLModel>& element, const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
        
        static const std::string BILLBOARD_VERTEX_SHADER;
        static const std::string BILLBOARD_FRAGMENT_SHADER;

        static const unsigned int TEXTURE_CACHE_SIZE;
        
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::weak_ptr<VectorLayer> _layer;
        std::weak_ptr<Options> _options;
    
        std::vector<std::shared_ptr<Billboard> > _elements;
        std::vector<std::shared_ptr<Billboard> > _tempElements;
        
        std::vector<std::shared_ptr<BillboardDrawData> > _drawDataBuffer;
        std::vector<std::shared_ptr<NMLModelDrawData> > _nmlDrawDataBuffer;
        
        std::vector<unsigned char> _colorBuf;
        std::vector<float> _coordBuf;
        std::vector<unsigned short> _indexBuf;
        std::vector<float> _texCoordBuf;
    
        std::shared_ptr<BitmapTextureCache> _textureCache;
        std::shared_ptr<Shader> _shader;
        GLuint _a_color;
        GLuint _a_coord;
        GLuint _a_texCoord;
        GLuint _u_mvpMat;
        GLuint _u_tex;
    
        std::shared_ptr<NMLResources> _nmlResources;
        std::map<std::weak_ptr<nml::Model>, std::shared_ptr<nml::GLModel>, std::owner_less<std::weak_ptr<nml::Model> > > _nmlModelMap;

        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
