#ifdef _CARTO_SEARCH_SUPPORT

#include "VectorTileSearchService.h"
#include "components/Exceptions.h"
#include "datasources/TileDataSource.h"
#include "geometry/Geometry.h"
#include "geometry/VectorTileFeature.h"
#include "geometry/VectorTileFeatureCollection.h"
#include "search/SearchProxy.h"
#include "vectortiles/VectorTileDecoder.h"
#include "projections/Projection.h"
#include "utils/TileUtils.h"
#include "utils/Log.h"

#include <vt/TileId.h>

namespace carto {

    VectorTileSearchService::VectorTileSearchService(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<VectorTileDecoder>& tileDecoder) :
        _dataSource(dataSource),
        _tileDecoder(tileDecoder),
        _minZoom(),
        _maxZoom(),
        _mutex()
    {
        if (!dataSource) {
            throw NullArgumentException("Null dataSource");
        }
        if (!tileDecoder) {
            throw NullArgumentException("Null tileDecoder");
        }

        _minZoom = _dataSource->getMinZoom();
        _maxZoom = _dataSource->getMaxZoom();
    }

    VectorTileSearchService::~VectorTileSearchService() {
    }

    const std::shared_ptr<TileDataSource>& VectorTileSearchService::getDataSource() const {
        return _dataSource;
    }

    const std::shared_ptr<VectorTileDecoder>& VectorTileSearchService::getTileDecoder() const {
        return _tileDecoder;
    }

    int VectorTileSearchService::getMinZoom() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _minZoom;
    }

    void VectorTileSearchService::setMinZoom(int minZoom) {
        std::lock_guard<std::mutex> lock(_mutex);
        _minZoom = minZoom;
    }

    int VectorTileSearchService::getMaxZoom() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _maxZoom;
    }

    void VectorTileSearchService::setMaxZoom(int maxZoom) {
        std::lock_guard<std::mutex> lock(_mutex);
        _maxZoom = maxZoom;
    }

    std::shared_ptr<VectorTileFeatureCollection> VectorTileSearchService::findFeatures(const std::shared_ptr<SearchRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        SearchProxy proxy(request, _dataSource->getDataExtent());
        MapBounds searchBounds = proxy.getSearchBounds();
        int minZoom = _dataSource->getMinZoom();
        int maxZoom = _dataSource->getMaxZoom();
        {
            std::lock_guard<std::mutex> lock(_mutex);
            minZoom = std::max(minZoom, _minZoom);
            maxZoom = std::min(maxZoom, _maxZoom);
        }

        std::vector<MapTile> mapTiles;
        for (int zoom = minZoom; zoom <= maxZoom; zoom++) {
            MapTile mapTile1 = TileUtils::CalculateMapTile(searchBounds.getMin(), zoom, _dataSource->getProjection());
            MapTile mapTile2 = TileUtils::CalculateMapTile(searchBounds.getMax(), zoom, _dataSource->getProjection());
            for (int y = std::min(mapTile1.getY(), mapTile2.getY()); y <= std::max(mapTile1.getY(), mapTile2.getY()); y++) {
                for (int x = std::min(mapTile1.getX(), mapTile2.getX()); x <= std::max(mapTile1.getX(), mapTile2.getX()); x++) {
                    MapTile mapTile(x, y, zoom, 0);
                    if (proxy.testBounds(TileUtils::CalculateMapTileBounds(mapTile, _dataSource->getProjection()), _dataSource->getProjection())) {
                        mapTiles.push_back(mapTile);
                    }
                }
            }
        }

        std::vector<std::shared_ptr<VectorTileFeature> > features;
        for (const MapTile& mapTile : mapTiles) {
            if (std::shared_ptr<TileData> tileData = _dataSource->loadTile(mapTile.getFlipped())) {
                MapBounds tileBounds = TileUtils::CalculateMapTileBounds(mapTile, _dataSource->getProjection());
                if (std::shared_ptr<VectorTileFeatureCollection> featureCollection = _tileDecoder->decodeFeatures(vt::TileId(mapTile.getZoom(), mapTile.getX(), mapTile.getY()), tileData->getData(), tileBounds)) {
                    for (int i = 0; i < featureCollection->getFeatureCount(); i++) {
                        const std::shared_ptr<VectorTileFeature>& feature = featureCollection->getFeature(i);
                        if (proxy.testElement(feature->getGeometry(), _dataSource->getProjection(), &feature->getLayerName(), feature->getProperties())) {
                            features.push_back(feature);
                        }
                    }
                }
            }
        }
        return std::make_shared<VectorTileFeatureCollection>(features);
    }

}

#endif
