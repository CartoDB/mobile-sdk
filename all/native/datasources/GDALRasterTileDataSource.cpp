#ifdef _CARTO_GDAL_SUPPORT

#include "GDALRasterTileDataSource.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "graphics/Bitmap.h"
#include "graphics/utils/BitmapFilterTable.h"
#include "utils/Log.h"

#include "assets/gdal/coordinate_axis_csv.h"
#include "assets/gdal/datum_shift_csv.h"
#include "assets/gdal/ellipsoid_csv.h"
#include "assets/gdal/gcs_csv.h"
#include "assets/gdal/gcs_override_csv.h"
#include "assets/gdal/gdal_datum_csv.h"
#include "assets/gdal/pcs_csv.h"
#include "assets/gdal/pcs_override_csv.h"
#include "assets/gdal/projop_wparm_csv.h"
#include "assets/gdal/unit_of_measure_csv.h"

#include <boost/lexical_cast.hpp>

#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <cpl_port.h>
#include <cpl_config.h>
#include <cpl_vsi.h>
#include <cpl_csv.h>

namespace {

    struct GDALVirtualFile {
        const char* fileName;
        unsigned char* contents;
        size_t size;
    };
    
    static const GDALVirtualFile _GDALVirtualFiles[] = {
        { "coordinate_axis.csv", _coordinate_axis_csv, sizeof(_coordinate_axis_csv) },
        { "datum_shift.csv", _datum_shift_csv, sizeof(_datum_shift_csv) },
        { "ellipsoid.csv", _ellipsoid_csv, sizeof(_ellipsoid_csv) },
        { "gcs.csv", _gcs_csv, sizeof(_gcs_csv) },
        { "gcs.override.csv", _gcs_override_csv, sizeof(_gcs_override_csv) },
        { "gdal_datum.csv", _gdal_datum_csv, sizeof(_gdal_datum_csv) },
        { "pcs.csv", _pcs_csv, sizeof(_pcs_csv) },
        { "pcs.override.csv", _pcs_override_csv, sizeof(_pcs_override_csv) },
        { "projop_wparm.csv", _projop_wparm_csv, sizeof(_projop_wparm_csv) },
        { "unit_of_measure.csv", _unit_of_measure_csv, sizeof(_unit_of_measure_csv) },
        { nullptr, nullptr, 0 }
    };

    struct GDALLibrary {
        GDALLibrary() {
            CPLSetErrorHandler(&GDALLibrary::ErrorHandler);
            
            // Redirect GDAL_DATA files to virtual memory file system, register all files required for projections to work
            CPLSetConfigOption("GDAL_DATA", "/vsimem");
            VSIInstallMemFileHandler();
            for (int i = 0; _GDALVirtualFiles[i].fileName; i++) {
                const GDALVirtualFile& vf = _GDALVirtualFiles[i];
                std::string fileName = std::string("/vsimem/") + vf.fileName;
                VSIFCloseL(VSIFileFromMemBuffer(fileName.c_str(), vf.contents, vf.size, FALSE));
            }
            
            // Register all data drivers
            GDALAllRegister();
        }

    private:
        static void ErrorHandler(CPLErr errClass, int errNo, const char* msg) {
            carto::Log::Errorf("GDALLibrary: Error %d, %s", errNo, msg);
        }
    } _GDALLibrary;

    struct AffineTransform {
        AffineTransform(const cglib::mat3x3<double>& matrix) : _matrix(matrix) { }

        cglib::vec2<double> operator() (int x, int y) const {
            return cglib::transform_point_affine(cglib::vec2<double>(x, y), _matrix);
        }

    private:
        cglib::mat3x3<double> _matrix;
    };
    
}

namespace carto {

    GDALRasterTileDataSource::GDALRasterTileDataSource(int minZoom, int maxZoom, const std::string& fileName) :
        TileDataSource(minZoom, maxZoom),
        _width(0),
        _height(0),
        _tileSize(256),
        _hasAlpha(false),
        _transform(cglib::mat3x3<double>::identity()),
        _invTransform(cglib::mat3x3<double>::identity()),
        _projection(std::make_shared<EPSG3857>()),
        _mutex()
    {
        _poDataset = (GDALDataset*)GDALOpen(fileName.c_str(), GA_ReadOnly);
        if (!_poDataset) {
            Log::Errorf("GDALRasterTileDataSource: Failed to open file %s.", fileName.c_str());
            return;
        }

        _width = _poDataset->GetRasterXSize();
        _height = _poDataset->GetRasterYSize();
        Log::Infof("GDALRasterTileDataSource: Width %d, height %d", _width, _height);
        
        std::shared_ptr<OGRSpatialReference> poDatasetSpatialRef = std::make_shared<OGRSpatialReference>();
        char* pWktDataset = const_cast<char*>(_poDataset->GetProjectionRef());
        if (poDatasetSpatialRef->importFromWkt(&pWktDataset) != OGRERR_NONE) {
            Log::Error("GDALRasterTileDataSource: Failed to read data set projection info");
        }

        initializeTransform(poDatasetSpatialRef);
    }
    
    GDALRasterTileDataSource::GDALRasterTileDataSource(int minZoom, int maxZoom, const std::string& fileName, const std::string& srs) :
        TileDataSource(minZoom, maxZoom),
        _width(0),
        _height(0),
        _tileSize(256),
        _hasAlpha(false),
        _transform(cglib::mat3x3<double>::identity()),
        _invTransform(cglib::mat3x3<double>::identity()),
        _projection(std::make_shared<EPSG3857>()),
        _mutex()
    {
        _poDataset = (GDALDataset*)GDALOpen(fileName.c_str(), GA_ReadOnly);
        if (!_poDataset) {
            Log::Errorf("GDALRasterTileDataSource: Failed to open file %s.", fileName.c_str());
            return;
        }
        
        _width = _poDataset->GetRasterXSize();
        _height = _poDataset->GetRasterYSize();
        Log::Infof("GDALRasterTileDataSource: Width %d, height %d", _width, _height);
        
        std::shared_ptr<OGRSpatialReference> poDatasetSpatialRef = std::make_shared<OGRSpatialReference>();
        if (srs.substr(0, 5) == "EPSG:") {
            int epsgCode = 3857;
            try {
                epsgCode = boost::lexical_cast<int>(srs.substr(5));
            }
            catch (const std::exception& ex) {
                Log::Errorf("GDALRasterTileDataSource: Could not parse EPSG code: %s", ex.what());
            }
            if (poDatasetSpatialRef->importFromEPSG(epsgCode) != OGRERR_NONE) {
                Log::Error("GDALRasterTileDataSource: Failed to import projection info");
            }
        } else {
            if (poDatasetSpatialRef->importFromProj4(srs.c_str()) != OGRERR_NONE) {
                Log::Error("GDALRasterTileDataSource: Failed to import PROJ4 projection info");
            }
        }
        
        initializeTransform(poDatasetSpatialRef);
    }
    
    GDALRasterTileDataSource::~GDALRasterTileDataSource() {
        if (_poDataset) {
            delete _poDataset;
        }
    }

    std::shared_ptr<TileData> GDALRasterTileDataSource::loadTile(const MapTile& mapTile) {
        if (!_poDataset) {
            return std::shared_ptr<TileData>();
        }

        // Calculate tile bounds
        MapBounds projBounds = _projection->getBounds();
        double scaleX =  projBounds.getDelta().getX() / (1 << mapTile.getZoom());
        double scaleY = -projBounds.getDelta().getY() / (1 << mapTile.getZoom());
        cglib::vec2<double> projP0(projBounds.getMin().getX(), projBounds.getMax().getY());
        cglib::vec2<double> tileP0(projP0(0) + scaleX * mapTile.getX(), projP0(1) + scaleY * mapTile.getY());

        // Calculate transform for tile pixel -> source pixel
        cglib::mat3x3<double> invTransform = _invTransform * cglib::translate3_matrix(cglib::vec3<double>(tileP0(0), tileP0(1), 1)) * cglib::scale3_matrix(cglib::vec3<double>(scaleX / _tileSize, scaleY / _tileSize, 1));

        // Find tile area in raster space
        int minU, minV, maxU, maxV;
        if (!BitmapFilterTable::calculateFilterBounds(AffineTransform(invTransform), _tileSize, _tileSize, _width, _height, minU, minV, maxU, maxV, MAX_FILTER_WIDTH)) {
            Log::Infof("GDALRasterTileDataSource: Tile %s outside of raster dataset", mapTile.toString().c_str());
            return std::shared_ptr<TileData>();
        }

        // Adjust downsampling factors. Downsampling allows to keep memory usage in control while degrading quality
        int downsampleU = 0;
        for (; downsampleU < 32; downsampleU++) {
            if ((maxU - minU) < _tileSize * (MAX_DOWNSAMPLE_FACTOR << downsampleU)) {
                break;
            }
        }
        int downsampleV = 0;
        for (; downsampleV < 32; downsampleV++) {
            if ((maxV - minV) < _tileSize * (MAX_DOWNSAMPLE_FACTOR << downsampleV)) {
                break;
            }
        }

        // Clip bounds, calculate downsampled bounds
        minU = std::max(minU, 0);
        minV = std::max(minV, 0);
        maxU = std::min(maxU, _width);
        maxV = std::min(maxV, _height);

        int minUds = minU >> downsampleU;
        int minVds = minV >> downsampleV;
        int maxUds = ((maxU - 1) >> downsampleU) + 1;
        int maxVds = ((maxV - 1) >> downsampleV) + 1;

        // Calculate tile pixel -> downsampled source pixel transform
        cglib::mat3x3<double> invTransformDS = cglib::scale3_matrix(cglib::vec3<double>(1.0 / (1 << downsampleU), 1.0 / (1 << downsampleV), 1)) * invTransform;

        // Calculate filter table
        Log::Infof("GDALRasterTileDataSource: Tile %s inside the raster dataset, extent %d,%d ... %d,%d, downsampling %d,%d", mapTile.toString().c_str(), minU, minV, maxU, maxV, downsampleU, downsampleV);
        BitmapFilterTable filterTable(minUds, minVds, maxUds, maxVds);
        filterTable.calculateFilterTable(AffineTransform(invTransformDS), _tileSize, _tileSize, FILTER_SCALE, MAX_FILTER_WIDTH);

        // Read tile data by band
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<unsigned char> data(_tileSize * _tileSize * 4);
        std::vector<unsigned char> bandData((maxUds - minUds) * (maxVds - minVds));
        for (int n = 1; n <= _poDataset->GetRasterCount(); n++) {
            GDALRasterBand* poRasterBand = _poDataset->GetRasterBand(n);
            if (!poRasterBand) {
                Log::Warnf("GDALRasterTileDataSource: Failed to read band %d", n);
                continue;
            }
            
            GDALColorInterp colorInterp = poRasterBand->GetColorInterpretation();
            int mask = 0;
            switch (colorInterp) {
            case GCI_GrayIndex:
                mask = 7;
                break;
            case GCI_RedBand:
                mask = 1;
                break;
            case GCI_GreenBand:
                mask = 2;
                break;
            case GCI_BlueBand:
                mask = 4;
                break;
            case GCI_AlphaBand:
                mask = 8;
                break;
            default:
                Log::Warnf("GDALRasterTileDataSource: Unsupported band %d, color interpretation %d", n, (int)colorInterp);
                break;
            }
            if (mask == 0) {
                continue;
            }

            poRasterBand->RasterIO(GF_Read, minU, minV, maxU - minU, maxV - minV, (void *)&bandData[0], maxUds - minUds, maxVds - minVds, GDT_Byte, 0, 0);

            size_t sampleIndex = 0;
            const std::vector<BitmapFilterTable::Sample>& samples = filterTable.getSamples();
            for (int i = 0; i < _tileSize * _tileSize; i++) {
                int count = filterTable.getSampleCounts()[i];
                if (count == 0) {
                    continue;
                }

                float filteredValue = 0.5f;
                for (int j = 0; j < count; j++) {
                    const BitmapFilterTable::Sample& sample = samples[sampleIndex++];
                    filteredValue += bandData[sample.v * (maxUds - minUds) + sample.u] * sample.weight;
                }
                for (int j = 0; mask >= (1 << j); j++) {
                    if (mask & (1 << j)) {
                        data[i * 4 + j] = static_cast<unsigned char>(filteredValue);
                    }
                }
            }
        }
        if (!_hasAlpha) {
            size_t sampleIndex = 0;
            const std::vector<BitmapFilterTable::Sample>& samples = filterTable.getSamples();
            for (int i = 0; i < _tileSize * _tileSize; i++) {
                int count = filterTable.getSampleCounts()[i];
                if (count == 0) {
                    continue;
                }
                
                float filteredValue = 0.5f;
                for (int j = 0; j < count; j++) {
                    const BitmapFilterTable::Sample& sample = samples[sampleIndex++];
                    filteredValue += 255 * sample.weight;
                }
                data[i * 4 + 3] = static_cast<unsigned char>(filteredValue);
            }
        }

        // Build bitmap, "compress" (serialize) to internal format
        Bitmap bitmap(data.data(), _tileSize, _tileSize, ColorFormat::COLOR_FORMAT_RGBA, 4 * _tileSize, false);
        return std::make_shared<TileData>(bitmap.compressToInternal());
    }

    MapBounds GDALRasterTileDataSource::getDataExtent() const {
        std::lock_guard<std::mutex> lock(_mutex);

        MapBounds bounds;
        for (int y = 0; y <= 1; y++) {
            for (int x = 0; x <= 1; x++) {
                cglib::vec2<double> p = cglib::transform_point_affine(cglib::vec2<double>(x * _width, y * _height), _transform);
                bounds.expandToContain(MapPos(p(0), p(1)));
            }
        }
        return bounds;
    }

    void GDALRasterTileDataSource::initializeTransform(const std::shared_ptr<OGRSpatialReference>& poDatasetSpatialRef) {
        std::shared_ptr<OGRSpatialReference> poEPSG3857SpatialRef = std::make_shared<OGRSpatialReference>();
        if (poEPSG3857SpatialRef->importFromEPSG(3857) != OGRERR_NONE) {
            Log::Error("GDALRasterTileDataSource: Failed to import EPSG3857");
        }

        std::shared_ptr<OGRCoordinateTransformation> poCoordinateTransform(OGRCreateCoordinateTransformation(poDatasetSpatialRef.get(), poEPSG3857SpatialRef.get()), OGRCoordinateTransformation::DestroyCT);

        double adfGeoTransform[6];
        if (_poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
            cglib::mat3x3<double> transform = cglib::mat3x3<double>::identity();
            transform(0, 0) = adfGeoTransform[1];
            transform(0, 1) = adfGeoTransform[2];
            transform(0, 2) = adfGeoTransform[0];
            transform(1, 0) = adfGeoTransform[4];
            transform(1, 1) = adfGeoTransform[5];
            transform(1, 2) = adfGeoTransform[3];
            
            if (!poEPSG3857SpatialRef->IsSame(poDatasetSpatialRef.get()) && poCoordinateTransform) {
                Log::Info("GDALRasterTileDataSource: Data source and file spatial reference systems do no match, using coordinate transformations.");
                cglib::vec2<double> p0 = cglib::transform_point(cglib::vec2<double>(0, 0), transform);
                cglib::vec2<double> px = cglib::transform_point(cglib::vec2<double>(_width, 0), transform);
                cglib::vec2<double> py = cglib::transform_point(cglib::vec2<double>(0, _height), transform);
                
                poCoordinateTransform->Transform(1, &p0(0), &p0(1));
                poCoordinateTransform->Transform(1, &px(0), &px(1));
                poCoordinateTransform->Transform(1, &py(0), &py(1));
                
                _transform(0, 0) = (px(0) - p0(0)) / _width;
                _transform(1, 0) = (px(1) - p0(1)) / _width;
                _transform(0, 1) = (py(0) - p0(0)) / _height;
                _transform(1, 1) = (py(1) - p0(1)) / _height;
                _transform(0, 2) = p0(0);
                _transform(1, 2) = p0(1);
            } else {
                _transform = transform;
            }
            
            _invTransform = cglib::inverse(_transform);
        } else {
            Log::Error("GDALRasterTileDataSource: Failed to read dataset transform.");
        }
        
        int rasterCount = _poDataset->GetRasterCount();
        Log::Infof("GDALRasterTileDataSource: Number of raster bands: %d", rasterCount);
        for (int n = 1; n <= rasterCount; n++) {
            GDALRasterBand* poRasterBand = _poDataset->GetRasterBand(n);
            if (!poRasterBand) {
                Log::Errorf("GDALRasterTileDataSource: Failed to read band %d", n);
                continue;
            }
            GDALDataType dataType = poRasterBand->GetRasterDataType();
            GDALColorInterp colorInterp = poRasterBand->GetColorInterpretation();
            Log::Infof("GDALRasterTileDataSource: Band %d, data type %d, color interpretation %d", n, (int)dataType, (int)colorInterp);
            if (colorInterp == GCI_AlphaBand) {
                _hasAlpha = true;
            }
        }
    }
    
    const float GDALRasterTileDataSource::FILTER_SCALE = 1.5f;
    const int GDALRasterTileDataSource::MAX_FILTER_WIDTH = 16;
    const int GDALRasterTileDataSource::MAX_DOWNSAMPLE_FACTOR = 8;
}

#endif
