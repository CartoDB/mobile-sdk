/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORTILESEARCHSERVICE_H_
#define _CARTO_VECTORTILESEARCHSERVICE_H_

#ifdef _CARTO_SEARCH_SUPPORT

#include "search/SearchRequest.h"

#include <memory>
#include <mutex>
#include <vector>

namespace carto {
    class Projection;
    class TileDataSource;
    class VectorTileDecoder;
    class VectorTileFeatureCollection;

    class VectorTileSearchService {
    public:
        VectorTileSearchService(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<VectorTileDecoder>& tileDecoder);
        virtual ~VectorTileSearchService();

        const std::shared_ptr<TileDataSource>& getDataSource() const;

        const std::shared_ptr<VectorTileDecoder>& getTileDecoder() const;

        int getMinZoom() const;
        void setMinZoom(int minZoom);

        int getMaxZoom() const;
        void setMaxZoom(int maxZoom);

        virtual std::shared_ptr<VectorTileFeatureCollection> findFeatures(const std::shared_ptr<SearchRequest>& request) const;

    protected:
        const std::shared_ptr<TileDataSource> _dataSource;
        const std::shared_ptr<VectorTileDecoder> _tileDecoder;

        int _minZoom;
        int _maxZoom;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
