/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_STROKEMAP_H_
#define _CARTO_VT_STROKEMAP_H_

#include "Bitmap.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace carto { namespace vt {
    class StrokeMap final {
    public:
        using StrokeId = unsigned int;
        
        struct Stroke {
            float scale;
            int y0;
            int y1;

            explicit Stroke(float scale, int y0, int y1) : scale(scale), y0(y0), y1(y1) { }
        };

        explicit StrokeMap(int width);

        int getWidth() const;
        const std::unique_ptr<const Stroke>& getStroke(StrokeId strokeId) const;
        StrokeId loadBitmapPattern(const std::shared_ptr<const BitmapPattern>& bitmapPattern);
        std::shared_ptr<const BitmapPattern> getBitmapPattern() const;

    private:
        const int _width;
        int _height;
        std::unordered_map<StrokeId, std::unique_ptr<const Stroke>> _strokeMap;
        std::unordered_map<std::shared_ptr<const BitmapPattern>, StrokeId> _bitmapStrokeMap;
        std::shared_ptr<BitmapPattern> _bitmapPattern;
        mutable std::mutex _mutex;
    };
} }

#endif
