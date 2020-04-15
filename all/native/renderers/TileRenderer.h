/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TILERENDERER_H_
#define _CARTO_TILERENDERER_H_

#include "graphics/Color.h"
#include "graphics/ViewState.h"
#include "renderers/utils/GLResource.h"

#include <memory>
#include <mutex>
#include <map>
#include <tuple>
#include <vector>

#include <cglib/ray.h>

#include <vt/TileId.h>
#include <vt/Tile.h>
#include <vt/Bitmap.h>
#include <vt/GLTileRenderer.h>

namespace carto {
    class Options;
    class MapRenderer;
    class GLResourceManager;
    class Shader;
    class TileDrawData;
    class ViewState;
    namespace vt {
        class LabelCuller;
        class TileTransformer;
        class GLTileRenderer;
    }
    
    class TileRenderer {
    public:
        TileRenderer(const std::shared_ptr<vt::TileTransformer>& tileTransformer);
        virtual ~TileRenderer();
    
        void setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer);
    
        void setInteractionMode(bool enabled);
        void setSubTileBlending(bool enabled);
        void setLabelOrder(int order);
        void setBuildingOrder(int order);

        void offsetLayerHorizontally(double offset);
    
        bool onDrawFrame(float deltaSeconds, const ViewState& viewState);
        bool onDrawFrame3D(float deltaSeconds, const ViewState& viewState);
    
        bool cullLabels(vt::LabelCuller& culler, const ViewState& viewState);

        bool refreshTiles(const std::vector<std::shared_ptr<TileDrawData> >& drawDatas);

        void calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, float radius, std::vector<std::tuple<vt::TileId, double, long long> >& results) const;
        void calculateRayIntersectedElements3D(const cglib::ray3<double>& ray, const ViewState& viewState, float radius, std::vector<std::tuple<vt::TileId, double, long long> >& results) const;
        void calculateRayIntersectedBitmaps(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<std::tuple<vt::TileId, double, vt::TileBitmap, cglib::vec2<float> > >& results) const;
    
    private:
        class GLBaseRenderer : public GLResource {
        public:
            virtual ~GLBaseRenderer();

            std::shared_ptr<vt::GLTileRenderer> get() const {
                std::lock_guard<std::mutex> lock(_mutex);
                return _renderer;
            }

        protected:
            friend GLResourceManager;

            GLBaseRenderer(const std::shared_ptr<GLResourceManager>& manager, const std::shared_ptr<vt::TileTransformer>& tileTransformer, const boost::optional<vt::GLTileRenderer::LightingShader>& lightingShader2D, const boost::optional<vt::GLTileRenderer::LightingShader>& lightingShader3D);

            virtual void create() const;
            virtual void destroy() const;

        private:
            const std::shared_ptr<vt::TileTransformer> _tileTransformer;
            const boost::optional<vt::GLTileRenderer::LightingShader> _lightingShader2D;
            const boost::optional<vt::GLTileRenderer::LightingShader> _lightingShader3D;

            mutable std::shared_ptr<vt::GLTileRenderer> _renderer;

            mutable std::mutex _mutex;
        };

        bool initializeRenderer();

        static const std::string LIGHTING_SHADER_2D;
        static const std::string LIGHTING_SHADER_3D;

        const std::shared_ptr<vt::TileTransformer> _tileTransformer;
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::weak_ptr<Options> _options;

        std::shared_ptr<GLBaseRenderer> _glBaseRenderer;

        bool _interactionMode;
        bool _subTileBlending;
        int _labelOrder;
        int _buildingOrder;
        double _horizontalLayerOffset;
        cglib::vec3<float> _viewDir;
        cglib::vec3<float> _mainLightDir;
        std::map<vt::TileId, std::shared_ptr<const vt::Tile> > _tiles;
        
        mutable std::mutex _mutex;
    };
    
}

#endif
