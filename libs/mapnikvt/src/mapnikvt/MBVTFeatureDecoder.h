/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_MBVTFEATUREDECODER_H_
#define _CARTO_MAPNIKVT_MBVTFEATUREDECODER_H_

#include "FeatureDecoder.h"

#include <memory>
#include <vector>
#include <map>
#include <unordered_set>

#include <cglib/bbox.h>
#include <cglib/mat.h>

namespace vector_tile {
    class Tile;
}

namespace carto { namespace mvt {
    class Logger;
    
    class MBVTFeatureDecoder : public FeatureDecoder {
    public:
        explicit MBVTFeatureDecoder(const std::vector<unsigned char>& data, std::shared_ptr<Logger> logger);

        void setTransform(const cglib::mat3x3<float>& transform);
        void setClipBox(const cglib::bbox2<float>& clipBox);
        void setBuffer(float buffer);

        std::shared_ptr<FeatureIterator> createLayerFeatureIterator(const std::string& name, const std::unordered_set<std::string>& fields) const;

    private:
        class MBVTFeatureIterator;

        static bool inflate(const std::vector<unsigned char>& in, std::vector<unsigned char>& out);
        
        cglib::mat3x3<float> _transform;
        float _buffer;
        cglib::bbox2<float> _clipBox;
        std::shared_ptr<vector_tile::Tile> _tile;
        std::map<std::string, int> _layerMap;
        mutable std::map<std::string, std::map<std::vector<int>, std::shared_ptr<FeatureData>>> _layerFeatureDataCache;

        const std::shared_ptr<Logger> _logger;
    };
} }

#endif
