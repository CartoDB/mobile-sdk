/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPTILE_H_
#define _CARTO_MAPTILE_H_

#include <string>
#include <utility>
#include <functional>

namespace carto {

    /**
     * An immutable map tile, used by tile layers for representing small pieces of map at different zoom levels and coordinates.
     */
    class MapTile {
    public:
        /**
         * Constructs a MapTile object based on coordinates, zoom level and frame number.
         * @param x The x coordinate of the tile.
         * @param y The y coordinate of the tile.
         * @param zoom The zoom level of the tile.
         * @param frameNr The frame number of the tile.
         */
        MapTile(int x, int y, int zoom, int frameNr);
    
        /**
         * Checks for equality between this and another map tile.
         * @param tile The other map tile.
         * @return True if equal.
         */
        bool operator ==(const MapTile& tile) const;
        /**
         * Checks for inequality between this and another map tile.
         * @param tile The other map tile.
         * @return True if not equal.
         */
        bool operator !=(const MapTile& tile) const;
    
        /**
         * Returns the x coordinate of this map tile.
         * @return The x coordinate of this map tile.
         */
        int getX() const;
        /**
         * Returns the y coordinate of this map tile.
         * @return The y coordinate of this map tile.
         */
        int getY() const;
        /**
         * Returns the zoom level of this map tile.
         * @return The zoom level of this map tile.
         */
        int getZoom() const;
        /**
         * Returns the time of this map tile.
         * @return The time of this map tile.
         */
        int getFrameNr() const;
    
        /**
         * Returns the internal tile id of this map tile.
         * @return The internal tile id of this map tile.
         */
        long long getTileId() const;
    
        /**
         * Get parent tile of the tile.
         * @return The parent tile of the tile, if zoom > 0. Otherwise tile itself.
         */
        MapTile getParent() const;

        /**
         * Get subtile of the tile.
         * @param index The index of the subtile. Must be between 0..3 (inclusive). Subtiles are ordered as top-left, top-right, bottom-left, bottom-right.
         * @return The specified subtile of the tile.
         */
        MapTile getChild(int index) const;

        /**
         * Get vertically flipped version of the tile.
         * @return The vertically flipped version of the tile.
         */
        MapTile getFlipped() const;

        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this map tile, useful for logging.
         * @return The string representation of this map tile.
         */
        std::string toString() const;
        
    protected:
        // Maximum range for all possible tile ids for a single raster layer
        static const long long TILE_ID_OFFSET;

        int _x;
        int _y;
        int _zoom;
        int _frameNr;
    
        long long _id;
    };
    
}

namespace std {

    template <>
    struct hash<carto::MapTile> {
        size_t operator() (const carto::MapTile& tile) const {
            return static_cast<size_t>(tile.hash());
        }
    };

}

#endif
