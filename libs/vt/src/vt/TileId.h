/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TILEID_H_
#define _CARTO_VT_TILEID_H_

#include <utility>
#include <functional>

namespace carto { namespace vt {
    struct TileId {
        int zoom;
        int x;
        int y;

        explicit TileId(int zoom, int x, int y) : zoom(zoom), x(x), y(y) { }

        TileId getParent() const {
            return TileId(zoom - 1, (x < 0 ? x - 1 : x) / 2, (y < 0 ? y - 1 : y) / 2);
        }

        TileId getChild(int dx, int dy) const {
            return TileId(zoom + 1, x * 2 + dx, y * 2 + dy);
        }

        bool intersects(const TileId& other) const {
            const TileId* tile1 = this;
            const TileId* tile2 = &other;
            if (tile2->zoom < tile1->zoom) {
                std::swap(tile1, tile2);
            }
            int deltaZoom = tile2->zoom - tile1->zoom;
            int minX = tile1->x << deltaZoom, maxX = (tile1->x + 1) << deltaZoom;
            int minY = tile1->y << deltaZoom, maxY = (tile1->y + 1) << deltaZoom;
            return minX <= tile2->x && maxX > tile2->x && minY <= tile2->y && maxY > tile2->y;
        }
    };

    inline bool operator == (const TileId& tile1, const TileId& tile2) {
        return tile1.zoom == tile2.zoom && tile1.x == tile2.x && tile1.y == tile2.y;
    }
        
    inline bool operator != (const TileId& tile1, const TileId& tile2) {
        return !(tile1 == tile2);
    }

    inline bool operator < (const TileId& tile1, const TileId& tile2) {
        if (tile1.zoom != tile2.zoom) {
            return tile1.zoom < tile2.zoom;
        }
        if (tile1.x != tile2.x) {
            return tile1.x < tile2.x;
        }
        return tile1.y < tile2.y;
    }
} }

namespace std {
    template <>
    struct hash<carto::vt::TileId> {
        std::size_t operator() (const carto::vt::TileId& tileId) const {
            return std::hash<long long>()(tileId.zoom + (((static_cast<long long>(tileId.x) << 32) + tileId.y) << 5));
        }
    };
}

#endif
