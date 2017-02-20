/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TORQUETILELAYER_H_
#define _CARTO_TORQUETILELAYER_H_

#include "layers/VectorTileLayer.h"

#include <memory>

namespace carto {
    class TorqueTileDecoder;

    /**
     * A vector tile layer for rendering time-based animated point clouds.
     */
    class TorqueTileLayer : public VectorTileLayer {
    public:
        /**
         * Constructs a new TorqueTileLayer object from a data source and decoder.
         * @param dataSource The data source from which this layer loads data.
         * @param decoder The tile decoder to use.
         */
        TorqueTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<TorqueTileDecoder>& decoder);
        virtual ~TorqueTileLayer();

        /**
         * Counts and returns the number of visible features (points) at the specified frame.
         * Note that this number depends on the view settings and may depend on the state of tiles. If not all tiles are loaded, the results could be off.
         * @param frameNr The frame number to use.
         * @return The number of visible feature at the specified frame. If the frame is missing, 0 is returned.
         */
        int countVisibleFeatures(int frameNr) const;
    };
}

#endif
