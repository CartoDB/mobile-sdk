#include "BitmapOverlayRasterTileDataSource.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "graphics/Bitmap.h"
#include "graphics/utils/BitmapFilterTable.h"
#include "utils/Log.h"

#include <array>

#include <cglib/mat.h>

namespace {

    struct ProjectiveTransform {
        ProjectiveTransform(const cglib::mat3x3<double>& matrix) : _matrix(matrix) { }

        cglib::vec2<double> operator() (int x, int y) const {
            return cglib::transform_point(cglib::vec2<double>(x, y), _matrix);
        }

    private:
        cglib::mat3x3<double> _matrix;
    };

}

namespace carto {

    BitmapOverlayRasterTileDataSource::BitmapOverlayRasterTileDataSource(int minZoom, int maxZoom, const std::shared_ptr<Bitmap>& bitmap, const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& mapPoses, const std::vector<ScreenPos>& bitmapPoses) :
        TileDataSource(minZoom, maxZoom),
        _tileSize(256),
        _origin(cglib::vec2<double>::zero()),
        _transform(cglib::mat3x3<double>::identity()),
        _invTransform(cglib::mat3x3<double>::identity()),
        _bitmap(),
        _projection(std::make_shared<EPSG3857>())
    {
        if (!bitmap) {
            throw NullArgumentException("Null bitmap");
        }
        if (!projection) {
            throw NullArgumentException("Null projection");
        }

        if (mapPoses.size() != 2 && mapPoses.size() != 3 && mapPoses.size() != 4) {
            throw InvalidArgumentException("Position arrays must contain 3 or 4 elements");
        }
        if (mapPoses.size() != bitmapPoses.size()) {
            throw InvalidArgumentException("Size mismatch between position arrays");
        }

        cglib::vec<double, 8> uvs = cglib::vec<double, 8>::zero();
        std::array<cglib::vec2<double>, 4> xys;
        for (std::size_t i = 0; i < mapPoses.size(); i++) {
            uvs(i * 2 + 0) = bitmapPoses[i].getX();
            uvs(i * 2 + 1) = bitmapPoses[i].getY();
            MapPos pos = mapPoses[i];
            if (!std::dynamic_pointer_cast<EPSG3857>(projection)) {
                pos = _projection->fromWgs84(projection->toWgs84(pos));
            }
            xys[i] = cglib::vec2<double>(pos.getX(), pos.getY());
            _origin += xys[i] * (1.0 / mapPoses.size());
        }

        cglib::mat<double, 8> posTransform = cglib::mat<double, 8>::zero();
        if (mapPoses.size() == 4) {
            // General case, perspective mapping. Must find 8 unknowns
            for (int i = 0; i < 8; i++) {
                int j = (i % 2) * 3;
                cglib::vec2<double> xy = xys[i / 2] - _origin;
                posTransform(i, j + 0) = xy(0);
                posTransform(i, j + 1) = xy(1);
                posTransform(i, j + 2) = 1;
                posTransform(i, 6) = -uvs(i) * xy(0);
                posTransform(i, 7) = -uvs(i) * xy(1);
            }
        } else if (mapPoses.size() == 3) {
            // Affine mapping. Must find 6 unknowns
            for (int i = 0; i < 6; i++) {
                int j = (i % 2) * 3;
                cglib::vec2<double> xy = xys[i / 2] - _origin;
                posTransform(i, j + 0) = xy(0);
                posTransform(i, j + 1) = xy(1);
                posTransform(i, j + 2) = 1;
            }
            posTransform(6, 6) = posTransform(7, 7) = 1;
        } else {
            // Conformal mapping. Must find 4 unknowns (rot + scale + transform)
            for (int n = 0; n < 2; n++) {
                int i = n * 2;
                cglib::vec2<double> xy = xys[n] - _origin;
                posTransform(i + 0, 0) = xy(0);
                posTransform(i + 0, 1) = -xy(1);
                posTransform(i + 0, 2) = 1;
                posTransform(i + 1, 0) = xy(1);
                posTransform(i + 1, 1) = xy(0);
                posTransform(i + 1, 3) = 1;
            }
            posTransform(4, 4) = posTransform(5, 5) = posTransform(6, 6) = posTransform(7, 7) = 1;
        }
        if (cglib::determinant(posTransform) == 0) {
            throw InvalidArgumentException("Map positions are collinear");
        }

        cglib::vec<double, 8> coeffs = cglib::transform(uvs, cglib::inverse(posTransform));
        if (mapPoses.size() == 2) {
            _invTransform(0, 0) = coeffs(0);
            _invTransform(0, 1) = -coeffs(1);
            _invTransform(1, 0) = coeffs(1);
            _invTransform(1, 1) = coeffs(0);
            _invTransform(0, 2) = coeffs(2);
            _invTransform(1, 2) = coeffs(3);
        } else {
            for (int i = 0; i < 8; i++) {
                _invTransform(i / 3, i % 3) = coeffs(i);
            }
        }
        _transform = cglib::inverse(_invTransform);

        _bitmap = bitmap;
        if (bitmap->getColorFormat() != ColorFormat::COLOR_FORMAT_RGBA) {
            _bitmap = bitmap->getRGBABitmap();
        }
    }

    BitmapOverlayRasterTileDataSource::~BitmapOverlayRasterTileDataSource() {
    }

    MapBounds BitmapOverlayRasterTileDataSource::getDataExtent() const {
        if (!_bitmap) {
            return MapBounds(MapPos(0, 0), MapPos(0, 0));
        }

        // Calculate map positions of 4 bitmap corners
        MapBounds bounds;
        for (int y = 0; y <= 1; y++) {
            for (int x = 0; x <= 1; x++) {
                cglib::vec2<double> p = _origin + cglib::transform_point_affine(cglib::vec2<double>(x * _bitmap->getWidth(), y * _bitmap->getHeight()), _transform);
                bounds.expandToContain(MapPos(p(0), p(1)));
            }
        }
        return bounds;
    }

    std::shared_ptr<TileData> BitmapOverlayRasterTileDataSource::loadTile(const MapTile& mapTile) {
        if (!_bitmap) {
            return std::shared_ptr<TileData>();
        }

        // Calculate tile bounds
        MapBounds projBounds = _projection->getBounds();
        double scaleX =  projBounds.getDelta().getX() / (1 << mapTile.getZoom());
        double scaleY = -projBounds.getDelta().getY() / (1 << mapTile.getZoom());
        cglib::vec2<double> projP0(projBounds.getMin().getX(), projBounds.getMax().getY());
        cglib::vec2<double> tileP0(projP0(0) - _origin(0) + scaleX * mapTile.getX(), projP0(1) - _origin(1) + scaleY * mapTile.getY());

        // Calculate transform for tile pixel -> source pixel
        cglib::mat3x3<double> invTransform = _invTransform * cglib::translate3_matrix(cglib::vec3<double>(tileP0(0), tileP0(1), 1)) * cglib::scale3_matrix(cglib::vec3<double>(scaleX / _tileSize, scaleY / _tileSize, 1));

        // Find tile area in raster space
        int minU, minV, maxU, maxV;
        if (!BitmapFilterTable::calculateFilterBounds(ProjectiveTransform(invTransform), _tileSize, _tileSize, _bitmap->getWidth(), _bitmap->getHeight(), minU, minV, maxU, maxV, MAX_FILTER_WIDTH)) {
            Log::Infof("BitmapOverlayRasterTileDataSource: Tile %s outside of bitmap", mapTile.toString().c_str());
            return std::shared_ptr<TileData>();
        }

        // Calculate filter table
        Log::Infof("BitmapOverlayRasterTileDataSource: Tile %s inside the raster dataset", mapTile.toString().c_str());
        BitmapFilterTable filterTable(0, 0, _bitmap->getWidth(), _bitmap->getHeight());
        filterTable.calculateFilterTable(ProjectiveTransform(invTransform), _tileSize, _tileSize, FILTER_SCALE, MAX_FILTER_WIDTH);
        
        std::size_t sampleIndex = 0;
        std::vector<unsigned char> data(_tileSize * _tileSize * 4);
        const std::vector<BitmapFilterTable::Sample>& samples = filterTable.getSamples();
        for (int i = 0; i < _tileSize * _tileSize; i++) {
            int count = filterTable.getSampleCounts()[i];
            if (count == 0) {
                continue;
            }

            float color[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
            for (int j = 0; j < count; j++) {
                const BitmapFilterTable::Sample& sample = samples[sampleIndex++];
                const unsigned char* sampleData = &_bitmap->getPixelData()[(sample.v * _bitmap->getWidth() + sample.u) * 4];
                for (int c = 0; c < 4; c++) {
                    color[c] += sampleData[c] * sample.weight;
                }
            }
            for (int c = 0; c < 4; c++) {
                data[i * 4 + c] = static_cast<unsigned char>(color[c]);
            }
        }

        // Build bitmap, "compress" (serialize) to internal format
        Bitmap bitmap(data.data(), _tileSize, _tileSize, ColorFormat::COLOR_FORMAT_RGBA, 4 * _tileSize);
        return std::make_shared<TileData>(bitmap.compressToInternal());
    }

    const float BitmapOverlayRasterTileDataSource::FILTER_SCALE = 1.5f;
    const int BitmapOverlayRasterTileDataSource::MAX_FILTER_WIDTH = 16;
}
