/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOMETRYCOLLECTIONRENDERER_H_
#define _CARTO_GEOMETRYCOLLECTIONRENDERER_H_

#include "renderers/LineRenderer.h"
#include "renderers/PointRenderer.h"
#include "renderers/PolygonRenderer.h"

#include <deque>
#include <memory>
#include <mutex>

namespace carto {
    class Bitmap;
    class GeometryCollection;
    class GeometryCollectionDrawData;
    class Projection;
    class Shader;
    class ShaderManager;
    class RayIntersectedElement;
    class VectorLayer;
    class ViewState;

    class GeometryCollectionRenderer {
    public:
        GeometryCollectionRenderer();
        virtual ~GeometryCollectionRenderer();

        virtual void offsetLayerHorizontally(double offset);

        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        void onDrawFrame(float deltaSeconds, StyleTextureCache& styleCache, const ViewState& viewState);
        void onSurfaceDestroyed();

        void addElement(const std::shared_ptr<GeometryCollection>& element);
        void refreshElements();
        void updateElement(const std::shared_ptr<GeometryCollection>& element);
        void removeElement(const std::shared_ptr<GeometryCollection>& element);

        virtual void calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;

    private:
        std::vector<std::shared_ptr<GeometryCollection> > _elements;
        std::vector<std::shared_ptr<GeometryCollection> > _tempElements;

        PointRenderer _pointRenderer;
        LineRenderer _lineRenderer;
        PolygonRenderer _polygonRenderer;

        mutable std::mutex _mutex;
    };

}

#endif
