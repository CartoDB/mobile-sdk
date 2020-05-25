#include "HillshadeRasterTileLayer.h"
#include "graphics/Bitmap.h"
#include "utils/Log.h"

#include <array>
#include <algorithm>

#include <vt/TileId.h>
#include <vt/Tile.h>
#include <vt/TileTransformer.h>
#include <vt/TileBitmap.h>
#include <vt/TileLayer.h>
#include <vt/TileLayerBuilder.h>
#include <vt/NormalMapBuilder.h>

namespace carto {

    HillshadeRasterTileLayer::HillshadeRasterTileLayer(const std::shared_ptr<TileDataSource>& dataSource) :
        RasterTileLayer(dataSource),
        _contrast(0.5f),
        _heightScale(1.0f)
    {
    }
    
    HillshadeRasterTileLayer::~HillshadeRasterTileLayer() {
    }
    
    float HillshadeRasterTileLayer::getContrast() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _contrast;
    }

    void HillshadeRasterTileLayer::setContrast(float contrast) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _contrast = std::min(1.0f, std::max(0.0f, contrast));
        }
        tilesChanged(false);
    }

    float HillshadeRasterTileLayer::getHeightScale() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _heightScale;
    }

    void HillshadeRasterTileLayer::setHeightScale(float heightScale) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _heightScale = heightScale;
        }
        tilesChanged(false);
    }

    std::shared_ptr<vt::Tile> HillshadeRasterTileLayer::createVectorTile(const MapTile& tile, const std::shared_ptr<Bitmap>& bitmap) const {
        std::uint8_t alpha = 0;
        std::array<float, 4> scales;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            alpha = static_cast<std::uint8_t>(_contrast * 255.0f);
            float exaggeration = tile.getZoom() < 2 ? 0.2f : tile.getZoom() < 5 ? 0.3f : 0.35f;
            float scale = 16 * _heightScale * static_cast<float>(bitmap->getHeight() * std::pow(2.0, tile.getZoom() * (1 - exaggeration)) / 40075016.6855785);
            scales = std::array<float, 4> { 65536 * scale, 256 * scale, scale, 0.0f };
        }
        
        // Build normal map from height map
        vt::TileId vtTileId(tile.getZoom(), tile.getX(), tile.getY());
        std::shared_ptr<Bitmap> rgbaBitmap = bitmap->getRGBABitmap();
        auto rgbaBitmapDataPtr = reinterpret_cast<const std::uint32_t*>(rgbaBitmap->getPixelData().data());
        std::vector<std::uint32_t> rgbaBitmapData(rgbaBitmapDataPtr, rgbaBitmapDataPtr + rgbaBitmap->getWidth() * rgbaBitmap->getHeight());
        auto vtBitmap = std::make_shared<vt::Bitmap>(rgbaBitmap->getWidth(), rgbaBitmap->getHeight(), std::move(rgbaBitmapData));
        vt::NormalMapBuilder normalMapBuilder(scales, alpha);
        std::shared_ptr<const vt::Bitmap> normalMap = normalMapBuilder.buildNormalMapFromHeightMap(vtTileId, vtBitmap);
        auto normalMapDataPtr = reinterpret_cast<const std::uint8_t*>(normalMap->data.data());
        std::vector<std::uint8_t> normalMapData(normalMapDataPtr, normalMapDataPtr + normalMap->data.size() * sizeof(std::uint32_t));
        auto tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Type::NORMALMAP, vt::TileBitmap::Format::RGBA, normalMap->width, normalMap->height, std::move(normalMapData));
        
        // Build vector tile from created normal map
        float tileSize = 256.0f; // 'normalized' tile size in pixels. Not really important
        std::shared_ptr<vt::TileBackground> tileBackground = std::make_shared<vt::TileBackground>(vt::Color(), std::shared_ptr<vt::BitmapPattern>());
        std::shared_ptr<const vt::TileTransformer::VertexTransformer> vtTransformer = getTileTransformer()->createTileVertexTransformer(vtTileId);
        vt::TileLayerBuilder tileLayerBuilder(vtTileId, 0, vtTransformer, tileSize, 1.0f); // Note: the size/scale argument is ignored
        tileLayerBuilder.addBitmap(tileBitmap);
        std::shared_ptr<vt::TileLayer> tileLayer = tileLayerBuilder.buildTileLayer(boost::optional<vt::CompOp>(), vt::FloatFunction(1));
        return std::make_shared<vt::Tile>(vtTileId, tileSize, tileBackground, std::vector<std::shared_ptr<vt::TileLayer> > { tileLayer });
    }

}
