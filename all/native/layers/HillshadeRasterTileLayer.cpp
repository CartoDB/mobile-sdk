#include "HillshadeRasterTileLayer.h"
#include "renderers/MapRenderer.h"
#include "renderers/TileRenderer.h"
#include "utils/Log.h"
#include "utils/TileUtils.h"
#include "core/BinaryData.h"
#include "projections/EPSG3857.h"
#include "projections/Projection.h"

#include <array>
#include <algorithm>

#include "graphics/Bitmap.h"

#include <vt/TileId.h>
#include <vt/Tile.h>
#include <vt/TileTransformer.h>
#include <vt/TileBitmap.h>
#include <vt/TileLayer.h>
#include <vt/TileLayerBuilder.h>
#include <vt/NormalMapBuilder.h>
#include <vt/TileLayerBuilder.h>

namespace {



    std::array<std::uint8_t, 4> readTileBitmapColor(const std::shared_ptr<carto::Bitmap>& bitmap, int x, int y) {
        x = std::max(0, std::min(x, (int)bitmap->getWidth() - 1));
        y = bitmap->getHeight() - 1 - std::max(0, std::min(y, (int)bitmap->getHeight() - 1));

        switch (bitmap->getColorFormat()) {
            case carto::ColorFormat::COLOR_FORMAT_GRAYSCALE:
            {
                std::uint8_t val = bitmap->getPixelData()[y * bitmap->getWidth() + x];
                return std::array<std::uint8_t, 4> { { val, val, val, 255 } };
            }
        case carto::ColorFormat::COLOR_FORMAT_RGB:
            {
                const std::uint8_t* valPtr = &bitmap->getPixelData()[(y * bitmap->getWidth() + x) * 3];
                return std::array<std::uint8_t, 4> { { valPtr[0], valPtr[1], valPtr[2], 255 } };
            }
        case  carto::ColorFormat::COLOR_FORMAT_RGBA:
            {
                const std::uint8_t* valPtr = &bitmap->getPixelData()[(y * bitmap->getWidth() + x) * 4];
                return std::array<std::uint8_t, 4> { { valPtr[0], valPtr[1], valPtr[2], valPtr[3] } };
            }
        default:
            break;
        }
        return std::array<std::uint8_t, 4> { { 0, 0, 0, 0 } };
    }

    std::array<std::uint8_t, 4> readTileBitmapColor(const std::shared_ptr<carto::Bitmap>& bitmap, float x, float y) {
        std::array<float, 4> result { { 0, 0, 0, 0 } };
        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
                int x0 = static_cast<int>(std::floor(x));
                int y0 = static_cast<int>(std::floor(y));

                std::array<std::uint8_t, 4> color = readTileBitmapColor(bitmap, x0 + dx, y0 + dy);
                for (int i = 0; i < 4; i++) {
                    result[i] += color[i] * (dx == 0 ? x0 + 1.0f - x : x - x0) * (dy == 0 ? y0 + 1.0f - y : y - y0);
                }
            }
        }
        return std::array<std::uint8_t, 4> { { static_cast<std::uint8_t>(result[0]), static_cast<std::uint8_t>(result[1]), static_cast<std::uint8_t>(result[2]), static_cast<std::uint8_t>(result[3]) } };
    }

    double readPixelAltitude(const std::shared_ptr<carto::Bitmap>& tileBitmap, const carto::MapBounds& tileBounds, const carto::MapPos& pos, const std::array<float, 4>& components) {
        int tileSize = tileBitmap->getWidth();
        float pixelX = (pos.getX() - tileBounds.getMin().getX()) / (tileBounds.getMax().getX() - tileBounds.getMin().getX()) * tileSize;
        float pixelY = tileSize - (pos.getY() - tileBounds.getMin().getY()) / (tileBounds.getMax().getY() - tileBounds.getMin().getY()) * tileSize;
        std::array<std::uint8_t, 4> interpolatedComponents = readTileBitmapColor(tileBitmap, pixelX - 0.5f, pixelY - 0.5f);
        int altitude = (double)(components[0] * (float)interpolatedComponents[0] + components[1] * (float)interpolatedComponents[1] + components[2] * (float)interpolatedComponents[2] + components[3] * (float)interpolatedComponents[3]/255.0f);
        return altitude;
    }
}

namespace carto
{

    HillshadeRasterTileLayer::HillshadeRasterTileLayer(const std::shared_ptr<TileDataSource> &dataSource, const std::shared_ptr<ElevationDecoder> &elevationDecoder) : RasterTileLayer(dataSource),
        _elevationDecoder(elevationDecoder),
        _contrast(0.5f),
        _heightScale(1.0f),
        _shadowColor(0, 0, 0, 255),
        _highlightColor(255, 255, 255, 255),
        _illuminationDirection(67),
        _illuminationMapRotationEnabled(true)
    {
    }

    HillshadeRasterTileLayer::~HillshadeRasterTileLayer()
    {
    }

    float HillshadeRasterTileLayer::getContrast() const
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _contrast;
    }

    void HillshadeRasterTileLayer::setContrast(float contrast)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _contrast = std::min(1.0f, std::max(0.0f, contrast));
        }
        tilesChanged(false);
    }

    float HillshadeRasterTileLayer::getHeightScale() const
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _heightScale;
    }

    void HillshadeRasterTileLayer::setHeightScale(float heightScale)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _heightScale = heightScale;
        }
        tilesChanged(false);
    }

    Color HillshadeRasterTileLayer::getShadowColor() const
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _shadowColor;
    }

    void HillshadeRasterTileLayer::setShadowColor(const Color &color)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _shadowColor = color;
        }
        redraw();
    }

    Color HillshadeRasterTileLayer::getHighlightColor() const
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _highlightColor;
    }

    void HillshadeRasterTileLayer::setHighlightColor(const Color &color)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _highlightColor = color;
        }
        redraw();
    }

    float HillshadeRasterTileLayer::getIlluminationDirection() const
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _illuminationDirection;
    }
    void HillshadeRasterTileLayer::setIlluminationDirection(float direction)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _illuminationDirection = direction;
        }
        redraw();
    }
    bool HillshadeRasterTileLayer::getIlluminationMapRotationEnabled() const
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _illuminationMapRotationEnabled;
    }
    void HillshadeRasterTileLayer::setIlluminationMapRotationEnabled(bool enabled)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _illuminationMapRotationEnabled = enabled;
        }
        redraw();
    }

    bool HillshadeRasterTileLayer::onDrawFrame(float deltaSeconds, BillboardSorter &billboardSorter, const ViewState &viewState)
    {
        updateTileLoadListener();

        if (auto mapRenderer = getMapRenderer())
        {
            float opacity = getOpacity();

            if (opacity < 1.0f)
            {
                mapRenderer->clearAndBindScreenFBO(Color(0, 0, 0, 0), false, false);
            }

            _tileRenderer->setRasterFilterMode(getRasterFilterMode());
            _tileRenderer->setNormalMapShadowColor(getShadowColor());
            _tileRenderer->setNormalMapHighlightColor(getHighlightColor());
            _tileRenderer->setNormalIlluminationDirection(getIlluminationDirection());
            _tileRenderer->setNormalIlluminationMapRotationEnabled(getIlluminationMapRotationEnabled());
            bool refresh = _tileRenderer->onDrawFrame(deltaSeconds, viewState);

            if (opacity < 1.0f)
            {
                mapRenderer->blendAndUnbindScreenFBO(opacity);
            }

            return refresh;
        }
        return false;
    }

    std::shared_ptr<vt::Tile> HillshadeRasterTileLayer::createVectorTile(const MapTile &tile, const std::shared_ptr<Bitmap> &bitmap) const
    {
        std::uint8_t alpha = 0;
        std::array<float, 4> scales;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            alpha = static_cast<std::uint8_t>(_contrast * 255.0f);
            float exaggeration = tile.getZoom() < 2 ? 0.2f : tile.getZoom() < 5 ? 0.3f : 0.35f;
            float scale = 16 * _heightScale * static_cast<float>(bitmap->getHeight() * std::pow(2.0, tile.getZoom() * (1 - exaggeration)) / 40075016.6855785);
            scales = _elevationDecoder->getVectorTileScales();
            std::transform(scales.begin(), scales.end(), scales.begin(), [&scale](float &c) { return c * scale; });
        }

        // Build normal map from height map
        vt::TileId vtTileId(tile.getZoom(), tile.getX(), tile.getY());
        std::shared_ptr<Bitmap> rgbaBitmap = bitmap->getRGBABitmap();
        auto rgbaBitmapDataPtr = reinterpret_cast<const std::uint32_t *>(rgbaBitmap->getPixelData().data());
        std::vector<std::uint32_t> rgbaBitmapData(rgbaBitmapDataPtr, rgbaBitmapDataPtr + rgbaBitmap->getWidth() * rgbaBitmap->getHeight());
        auto vtBitmap = std::make_shared<vt::Bitmap>(rgbaBitmap->getWidth(), rgbaBitmap->getHeight(), std::move(rgbaBitmapData));
        vt::NormalMapBuilder normalMapBuilder(scales, alpha);
        std::shared_ptr<const vt::Bitmap> normalMap = normalMapBuilder.buildNormalMapFromHeightMap(vtTileId, vtBitmap);
        auto normalMapDataPtr = reinterpret_cast<const std::uint8_t *>(normalMap->data.data());
        std::vector<std::uint8_t> normalMapData(normalMapDataPtr, normalMapDataPtr + normalMap->data.size() * sizeof(std::uint32_t));
        auto tileBitmap = std::make_shared<vt::TileBitmap>(vt::TileBitmap::Type::NORMALMAP, vt::TileBitmap::Format::RGBA, normalMap->width, normalMap->height, std::move(normalMapData));

        // Build vector tile from created normal map
        float tileSize = 256.0f; // 'normalized' tile size in pixels. Not really important
        std::shared_ptr<vt::TileBackground> tileBackground = std::make_shared<vt::TileBackground>(vt::Color(), std::shared_ptr<vt::BitmapPattern>());
        std::shared_ptr<const vt::TileTransformer::VertexTransformer> vtTransformer = getTileTransformer()->createTileVertexTransformer(vtTileId);
        vt::TileLayerBuilder tileLayerBuilder(vtTileId, 0, vtTransformer, tileSize, 1.0f); // Note: the size/scale argument is ignored
        tileLayerBuilder.addBitmap(tileBitmap);
        std::shared_ptr<vt::TileLayer> tileLayer = tileLayerBuilder.buildTileLayer(boost::optional<vt::CompOp>(), vt::FloatFunction(1));
        return std::make_shared<vt::Tile>(vtTileId, tileSize, tileBackground, std::vector<std::shared_ptr<vt::TileLayer>>{tileLayer});
    }


    std::shared_ptr<Bitmap> HillshadeRasterTileLayer::getMapTileBitmap(const MapTile& mapTile) const {
        std::shared_ptr<TileData> tileData = _dataSource->loadTile(mapTile);
        if (!tileData) {
            Log::Error("HillshadeRasterTileLayer::getMapTileBitmap: Null tile data");
            return NULL;
        }

        std::shared_ptr<BinaryData> binaryData = tileData->getData();
        if (!binaryData) {
            Log::Error("HillshadeRasterTileLayer::getMapTileBitmap: Null tile binary data");
            return NULL;
        }
        int size = binaryData->size();
        std::shared_ptr<Bitmap> tileBitmap = Bitmap::CreateFromCompressed(binaryData);
        return tileBitmap;
    }

    double HillshadeRasterTileLayer::getElevation(const MapPos &pos) const
    {
        std::shared_ptr<TileDataSource> dataSource = getDataSource();
        // we need to transform pos to dataSource projection
        // TODO: how to check if pos is in Wgs84?
        std::shared_ptr<Projection> projection = dataSource->getProjection();
        MapPos dataSourcePos = projection->fromWgs84(pos);

        // The tile is flipped so to get the bitmap we need to flip it
        MapTile mapTile = TileUtils::CalculateMapTile(dataSourcePos, dataSource->getMaxZoom(), projection);
        MapTile flippedMapTile = mapTile.getFlipped();

        std::shared_ptr<Bitmap> tileBitmap = getMapTileBitmap(flippedMapTile);
        if (!tileBitmap) {
            Log::Error("ElevationDecoder::getElevation: Null tile bitmap");
            return -1000000;
        }
        std::array<float, 4> components = _elevationDecoder->getColorComponentCoefficients();
        return readPixelAltitude(tileBitmap, TileUtils::CalculateMapTileBounds(mapTile, projection), dataSourcePos, components);
    }
    std::vector<double> HillshadeRasterTileLayer::getElevations(const std::vector<MapPos> poses) const
    {
        std::shared_ptr<TileDataSource> dataSource = getDataSource();
        std::map<long long, std::pair<MapBounds, std::shared_ptr<Bitmap>>> indexedTiles;
        std::vector<double> results;
        std::shared_ptr<Projection> projection = dataSource->getProjection();
        std::array<float, 4> components = _elevationDecoder->getColorComponentCoefficients();
        for (auto it = poses.begin(); it != poses.end(); it++) {
            // TODO: how to check if pos is in Wgs84?
            MapPos dataSourcePos = projection->fromWgs84(*it);
            // The tile is flipped so to get the bitmap we need to flip it
            MapTile mapTile = TileUtils::CalculateMapTile(dataSourcePos, dataSource->getMaxZoom(), projection);
            MapTile flippedMapTile = mapTile.getFlipped();
            long long tileId = mapTile.getTileId();
            std::map<long long, std::pair<MapBounds, std::shared_ptr<Bitmap>>>::iterator iter(indexedTiles.find(tileId));
            if (iter == indexedTiles.end()) {
                std::shared_ptr<Bitmap> tileBitmap = getMapTileBitmap(flippedMapTile);
                MapBounds tileBounds = TileUtils::CalculateMapTileBounds(mapTile, projection);
                std::pair<MapBounds, std::shared_ptr<Bitmap>> pair = std::make_pair(tileBounds, tileBitmap);
                indexedTiles.insert(std::pair<long long, std::pair<MapBounds, std::shared_ptr<Bitmap>>>(tileId, pair));
                double altitude = readPixelAltitude(tileBitmap, tileBounds, dataSourcePos, components);
                results.push_back(altitude);
            } else {
                std::pair<MapBounds, std::shared_ptr<Bitmap>> pair = iter->second;
                const std::shared_ptr<Bitmap>& tileBitmap = pair.second;
                const MapBounds& tileBounds = pair.first;
                double altitude = readPixelAltitude(tileBitmap, tileBounds, dataSourcePos, components);
                results.push_back(altitude);
            }
        }
        return results;
    }
} // namespace carto
