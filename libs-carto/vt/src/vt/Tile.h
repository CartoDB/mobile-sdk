/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TILE_H_
#define _CARTO_VT_TILE_H_

#include "TileId.h"
#include "TileLayer.h"

#include <memory>
#include <vector>
#include <numeric>

namespace carto { namespace vt {
    class Tile final {
    public:
        explicit Tile(const TileId& tileId, std::vector<std::shared_ptr<TileLayer>> layers) : _tileId(tileId), _layers(std::move(layers)) { }

        const TileId& getTileId() const { return _tileId; }
        const std::vector<std::shared_ptr<TileLayer>>& getLayers() const { return _layers; }

        std::size_t getResidentSize() const {
            std::size_t layersSize = std::accumulate(_layers.begin(), _layers.end(), static_cast<std::size_t>(0), [](std::size_t size, const std::shared_ptr<TileLayer>& layer) { return size + layer->getResidentSize(); });
            return 16 + layersSize;
        }

    private:
        const TileId _tileId;
        const std::vector<std::shared_ptr<TileLayer>> _layers;
    };
} }

#endif
