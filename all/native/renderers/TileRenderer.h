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
#include <vt/Styles.h>
#include <vt/GLTileRenderer.h>

namespace carto {
    class Options;
    class MapRenderer;
    class TileDrawData;
    class ViewState;
    class VTRenderer;
    namespace vt {
        class LabelCuller;
        class TileTransformer;
    }
    
    class TileRenderer {
    public:
        TileRenderer();
        virtual ~TileRenderer();
    
        void setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer);

        std::shared_ptr<vt::TileTransformer> getTileTransformer() const;
        void setTileTransformer(const std::shared_ptr<vt::TileTransformer>& tileTransformer);
    
        void setInteractionMode(bool enabled);
        void setSubTileBlending(bool enabled);
        void setLabelOrder(int order);
        void setBuildingOrder(int order);
        void setRasterFilterMode(vt::RasterFilterMode filterMode);
        void setNormalMapShadowColor(const Color& color);
        void setNormalMapHighlightColor(const Color& color);

        void offsetLayerHorizontally(double offset);
    
        bool onDrawFrame(float deltaSeconds, const ViewState& viewState);
        bool onDrawFrame3D(float deltaSeconds, const ViewState& viewState);
    
        bool cullLabels(vt::LabelCuller& culler, const ViewState& viewState);

        bool refreshTiles(const std::vector<std::shared_ptr<TileDrawData> >& drawDatas);

        void calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, float radius, std::vector<vt::GLTileRenderer::GeometryIntersectionInfo>& results) const;
        void calculateRayIntersectedElements3D(const cglib::ray3<double>& ray, const ViewState& viewState, float radius, std::vector<vt::GLTileRenderer::GeometryIntersectionInfo>& results) const;
        void calculateRayIntersectedBitmaps(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<vt::GLTileRenderer::BitmapIntersectionInfo>& results) const;
    
    private:
        bool initializeRenderer();

        static const std::string LIGHTING_SHADER_2D;
        static const std::string LIGHTING_SHADER_3D;
        static const std::string LIGHTING_SHADER_NORMALMAP;

        std::weak_ptr<MapRenderer> _mapRenderer;
        std::weak_ptr<Options> _options;
        std::shared_ptr<vt::TileTransformer> _tileTransformer;

        std::shared_ptr<VTRenderer> _vtRenderer;
        bool _interactionMode;
        bool _subTileBlending;
        int _labelOrder;
        int _buildingOrder;
        vt::RasterFilterMode _rasterFilterMode;
        Color _normalMapShadowColor;
        Color _normalMapHighlightColor;
        double _horizontalLayerOffset;
        cglib::vec3<float> _viewDir;
        cglib::vec3<float> _mainLightDir;
        std::map<vt::TileId, std::shared_ptr<const vt::Tile> > _tiles;
        
        mutable std::mutex _mutex;
    };
    
}

#endif
