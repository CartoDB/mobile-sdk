/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TILELAYER_H_
#define _CARTO_VT_TILELAYER_H_

#include "TileBitmap.h"
#include "TileGeometry.h"
#include "TileLabel.h"

#include <memory>
#include <string>
#include <vector>
#include <numeric>

#include <boost/optional.hpp>

namespace carto { namespace vt {
    class TileLayer final {
    public:
        explicit TileLayer(int layerIdx, std::shared_ptr<FloatFunction> opacity, boost::optional<CompOp> compOp, std::vector<std::shared_ptr<TileBitmap>> bitmaps, std::vector<std::shared_ptr<TileGeometry>> geometries, std::vector<std::shared_ptr<TileLabel>> labels) : _layerIdx(layerIdx), _opacity(std::move(opacity)), _compOp(std::move(compOp)), _bitmaps(std::move(bitmaps)), _geometries(std::move(geometries)), _labels(std::move(labels)) { }

        int getLayerIndex() const { return _layerIdx; }
        std::shared_ptr<FloatFunction> getOpacity() const { return _opacity; }
        boost::optional<CompOp> getCompOp() const { return _compOp; }

        const std::vector<std::shared_ptr<TileBitmap>>& getBitmaps() const { return _bitmaps; }
        const std::vector<std::shared_ptr<TileGeometry>>& getGeometries() const { return _geometries; }
        const std::vector<std::shared_ptr<TileLabel>>& getLabels() const { return _labels; }

        std::size_t getResidentSize() const {
            std::size_t bitmapSize = std::accumulate(_bitmaps.begin(), _bitmaps.end(), static_cast<std::size_t>(0), [](std::size_t size, const std::shared_ptr<TileBitmap>& bitmap) { return size + bitmap->getResidentSize(); });
            std::size_t geometriesSize = std::accumulate(_geometries.begin(), _geometries.end(), static_cast<std::size_t>(0), [](std::size_t size, const std::shared_ptr<TileGeometry>& geometry) { return size + geometry->getResidentSize(); });
            std::size_t labelsSize = _labels.size() * sizeof(TileLabel);
            return 16 + bitmapSize + geometriesSize + labelsSize;
        }

    private:
        const int _layerIdx;
        const std::shared_ptr<FloatFunction> _opacity;
        const boost::optional<CompOp> _compOp;
        const std::vector<std::shared_ptr<TileBitmap>> _bitmaps;
        const std::vector<std::shared_ptr<TileGeometry>> _geometries;
        const std::vector<std::shared_ptr<TileLabel>> _labels;
    };
} }

#endif
