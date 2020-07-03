/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPBOXELEVATIONDATADECODER_H_
#define _CARTO_MAPBOXELEVATIONDATADECODER_H_

#include "rastertiles/ElevationDecoder.h"

#include <memory>
#include <mutex>
#include <map>
#include <string>

namespace carto {

    /**
     * A decoder for MapBox encoded elevation tiles
     */
    class MapBoxElevationDataDecoder : public ElevationDecoder {
    public:
        /**
         * Constructs a new MapBoxElevationDataDecoder.
         */
        MapBoxElevationDataDecoder();
        virtual ~MapBoxElevationDataDecoder();

        virtual std::array<float, 4> getVectorTileScales() const;
        virtual std::array<float, 4> getColorComponentCoefficients() const;

    private :
        static const std::array<float, 4> COMPONENTS;
        static const std::array<float, 4> SCALES;
    };

}

#endif
