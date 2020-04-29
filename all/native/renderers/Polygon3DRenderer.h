/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGON3DRENDERER_H_
#define _CARTO_POLYGON3DRENDERER_H_

#include "renderers/utils/GLContext.h"

#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#include <cglib/ray.h>

namespace carto {
    class Polygon3D;
    class Polygon3DDrawData;
    class Options;
    class MapRenderer;
    class Shader;
    class RayIntersectedElement;
    class VectorLayer;
    class ViewState;
    
    class Polygon3DRenderer {
    public:
        Polygon3DRenderer();
        virtual ~Polygon3DRenderer();
        
        void setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer);
    
        void offsetLayerHorizontally(double offset);
    
        void onDrawFrame(float deltaSeconds, const ViewState& viewState);
    
        void addElement(const std::shared_ptr<Polygon3D>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<Polygon3D>& element);
        void removeElement(const std::shared_ptr<Polygon3D>& element);
        
        void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
    
    private:
        static void BuildAndDrawBuffers(GLuint a_color,
                                        GLuint a_attrib,
                                        GLuint a_coord,
                                        GLuint a_normal,
                                        std::vector<unsigned char>& colorBuf,
                                        std::vector<unsigned char>& attribBuf,
                                        std::vector<float>& coordBuf,
                                        std::vector<float>& normalBuf,
                                        std::vector<std::shared_ptr<Polygon3DDrawData> >& drawDataBuffer,
                                        const ViewState& viewState);
        
        bool initializeRenderer();
        void drawBatch(const ViewState& viewState);
        
        static const std::string POLYGON3D_VERTEX_SHADER;
        static const std::string POLYGON3D_FRAGMENT_SHADER;
    
        std::weak_ptr<Options> _options;
        std::weak_ptr<MapRenderer> _mapRenderer;
        
        std::vector<std::shared_ptr<Polygon3D> > _elements;
        std::vector<std::shared_ptr<Polygon3D> > _tempElements;
        
        std::vector<std::shared_ptr<Polygon3DDrawData> > _drawDataBuffer;
    
        std::vector<unsigned char> _colorBuf;
        std::vector<unsigned char> _attribBuf;
        std::vector<float> _coordBuf;
        std::vector<float> _normalBuf;
    
        std::shared_ptr<Shader> _shader;
        GLuint _a_color;
        GLuint _a_attrib;
        GLuint _a_coord;
        GLuint _a_normal;
        GLuint _u_ambientColor;
        GLuint _u_lightColor;
        GLuint _u_lightDir;
        GLuint _u_mvpMat;
    
        mutable std::mutex _mutex;
    };
    
}

#endif
