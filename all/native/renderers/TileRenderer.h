/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TILERENDERER_H_
#define _CARTO_TILERENDERER_H_

#include "graphics/Color.h"
#include "graphics/ViewState.h"

#include <memory>
#include <mutex>
#include <map>
#include <tuple>
#include <vector>

#include <cglib/ray.h>

#include <vt/TileId.h>
#include <vt/Tile.h>
#include <vt/Bitmap.h>

namespace carto {
    class Options;
    class Shader;
    class ShaderManager;
    class TextureManager;
    class TileDrawData;
    class MapRenderer;
    class ViewState;
    namespace vt {
        class TileTransformer;
        class GLTileRenderer;
    }
    
    class TileRenderer : public std::enable_shared_from_this<TileRenderer> {
    public:
        TileRenderer(const std::weak_ptr<MapRenderer>& mapRenderer, const std::shared_ptr<vt::TileTransformer>& tileTransformer);
        virtual ~TileRenderer();
    
        void setOptions(const std::weak_ptr<Options>& options);
    
        void setInteractionMode(bool enabled);
        void setSubTileBlending(bool enabled);
        void setLabelOrder(int order);
        void setBuildingOrder(int order);

        void offsetLayerHorizontally(double offset);
    
        void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        bool onDrawFrame(float deltaSeconds, const ViewState& viewState);
        bool onDrawFrame3D(float deltaSeconds, const ViewState& viewState);
        void onSurfaceDestroyed();
    
        bool cullLabels(const ViewState& viewState);

        bool refreshTiles(const std::vector<std::shared_ptr<TileDrawData> >& drawDatas);

        void calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<std::tuple<vt::TileId, double, long long> >& results) const;
        void calculateRayIntersectedElements3D(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<std::tuple<vt::TileId, double, long long> >& results) const;
        void calculateRayIntersectedBitmaps(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<std::tuple<vt::TileId, double, vt::TileBitmap, cglib::vec2<float> > >& results) const;
    
    private:
        static const int CLICK_RADIUS = 4;

        static const std::string LIGHTING_SHADER_2D;
        static const std::string LIGHTING_SHADER_3D;

        std::weak_ptr<MapRenderer> _mapRenderer;
        std::shared_ptr<vt::TileTransformer> _tileTransformer;
        std::shared_ptr<vt::GLTileRenderer> _glRenderer;
        bool _interactionMode;
        bool _subTileBlending;
        int _labelOrder;
        int _buildingOrder;
        double _horizontalLayerOffset;
        cglib::vec3<float> _lightDir;
        std::map<vt::TileId, std::shared_ptr<const vt::Tile> > _tiles;

        std::weak_ptr<Options> _options;
        
        mutable std::mutex _mutex;
    };
    
}

#endif
