#include "ElevationDecoder.h"
#include "datasources/TileDataSource.h"
#include "utils/Log.h"
#include "utils/TileUtils.h"
#include "core/MapBounds.h"
#include "core/MapTile.h"
#include "graphics/Bitmap.h"
#include "projections/EPSG3857.h"
#include "projections/Projection.h"

#include <algorithm>
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

    int readPixelAltitude(const std::shared_ptr<carto::Bitmap>& tileBitmap, const carto::MapBounds& tileBounds, const carto::MapPos& pos, const std::array<float, 4>& components) {
        int tileSize = tileBitmap->getWidth();
        float pixelX = (pos.getX() - tileBounds.getMin().getX()) / (tileBounds.getMax().getX() - tileBounds.getMin().getX()) * tileSize;
        float pixelY = tileSize - (pos.getY() - tileBounds.getMin().getY()) / (tileBounds.getMax().getY() - tileBounds.getMin().getY()) * tileSize;
        std::array<std::uint8_t, 4> interpolatedComponents = readTileBitmapColor(tileBitmap, (int)pixelX, (int)pixelY);
        int altitude = std::round(components[0] * (float)interpolatedComponents[0] + components[1] * (float)interpolatedComponents[1] + components[2] * (float)interpolatedComponents[2] + components[3] * (float)interpolatedComponents[3]/255.0f);
        return altitude;
    }
}
namespace carto
{
    ElevationDecoder::~ElevationDecoder()
    {
    }

    ElevationDecoder::ElevationDecoder()
    {
    }
    float ElevationDecoder::decodeHeight(const Color &encodedHeight) const
    {
        // the 4th component is used to "translate" the result
        const std::array<float, 4> components = getColorComponentCoefficients();
        return components[0] * encodedHeight.getR() + components[1] * encodedHeight.getG() + components[2] * encodedHeight.getB() + components[3] * encodedHeight.getA();
    }
    std::shared_ptr<Bitmap> ElevationDecoder::getMapTileBitmap(std::shared_ptr<TileDataSource> dataSource, const MapTile& mapTile) const {
        std::shared_ptr<TileData> tileData = dataSource->loadTile(mapTile);
        if (!tileData) {
            Log::Error("ElevationDecoder::getMapTileBitmap: Null tile data");
            return NULL;
        }

        std::shared_ptr<BinaryData> binaryData = tileData->getData();
        if (!binaryData) {
            Log::Error("ElevationDecoder::getMapTileBitmap: Null tile binary data");
            return NULL;
        }
        std::shared_ptr<Bitmap> tileBitmap = Bitmap::CreateFromCompressed(binaryData);
        return tileBitmap;
    }
    int ElevationDecoder::getElevation(std::shared_ptr<TileDataSource> dataSource, const MapPos &pos) const
    {
        // we need to transform pos to dataSource projection
        // TODO: how to check if pos is in Wgs84?
        std::shared_ptr<Projection> projection = dataSource->getProjection();
        MapPos dataSourcePos = projection->fromWgs84(pos);

        // The tile is flipped so to get the bitmap we need to flip it
        MapTile mapTile = TileUtils::CalculateMapTile(dataSourcePos, dataSource->getMaxZoom(), projection);
        MapTile flippedMapTile = mapTile.getFlipped();

        std::shared_ptr<Bitmap> tileBitmap = getMapTileBitmap(dataSource, flippedMapTile);
        if (!tileBitmap) {
            Log::Error("ElevationDecoder::getElevation: Null tile bitmap");
            return -1000000;
        }
        std::array<float, 4> components = getColorComponentCoefficients();
        return readPixelAltitude(tileBitmap, TileUtils::CalculateMapTileBounds(mapTile, projection), dataSourcePos, components);
    }
    std::vector<int> ElevationDecoder::getElevations(std::shared_ptr<TileDataSource> dataSource, const std::vector<MapPos> poses) const
    {
        std::map<long long, std::pair<MapBounds, std::shared_ptr<Bitmap>>> indexedTiles;
        std::vector<int> results;
        std::shared_ptr<Projection> projection = dataSource->getProjection();
        std::array<float, 4> components = getColorComponentCoefficients();
        for (auto it = poses.begin(); it != poses.end(); it++) {
            // TODO: how to check if pos is in Wgs84?
            MapPos dataSourcePos = projection->fromWgs84(*it);
            // The tile is flipped so to get the bitmap we need to flip it
            MapTile mapTile = TileUtils::CalculateMapTile(dataSourcePos, dataSource->getMaxZoom(), projection);
            MapTile flippedMapTile = mapTile.getFlipped();
            long long tileId = mapTile.getTileId();
            std::map<long long, std::pair<MapBounds, std::shared_ptr<Bitmap>>>::iterator iter(indexedTiles.lower_bound(tileId));
            Log::Debugf("ElevationDecoder::getElevations: %d, %d, %d, %d",it->getY(),it->getY(), tileId, iter == indexedTiles.end());
            if (iter == indexedTiles.end()) {
                std::shared_ptr<Bitmap> tileBitmap = getMapTileBitmap(dataSource, flippedMapTile);
                MapBounds tileBounds = TileUtils::CalculateMapTileBounds(mapTile, projection);
                std::pair<MapBounds, std::shared_ptr<Bitmap>> pair = std::make_pair(tileBounds, tileBitmap);
                indexedTiles.insert(std::pair<long long, std::pair<MapBounds, std::shared_ptr<Bitmap>>>(iter->first, pair));
                int altitude = readPixelAltitude(tileBitmap, tileBounds, dataSourcePos, components);
                results.push_back(altitude);
            } else {
                std::pair<MapBounds, std::shared_ptr<Bitmap>> pair = iter->second;
                const std::shared_ptr<Bitmap>& tileBitmap = pair.second;
                const MapBounds& tileBounds = pair.first;
                int altitude = readPixelAltitude(tileBitmap, tileBounds, dataSourcePos, components);
                results.push_back(altitude);
            }
        }
        return results;
    }
} // namespace carto
