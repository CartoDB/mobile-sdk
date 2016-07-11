/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TILEDATA_H_
#define _CARTO_TILEDATA_H_

#include <chrono>
#include <memory>
#include <mutex>
#include <vector>

namespace carto {
    class BinaryData;
    
    /**
     * A wrapper class for tile data.
     */
    class TileData {
    public:
        /**
         * Constructs a TileData object from a data blob.
         * @param data The source tile data.
         */
        TileData(const std::shared_ptr<BinaryData>& data);
        virtual ~TileData();
        
        /**
         * Returns the maximum age of the tile data, tile data will expire after that point.
         * @return Tile data maximum age in milliseconds, or -1 if the data does not expire.
         */
        long long getMaxAge() const;
        /**
         * Sets the maximum age of tile data, tile data will expire after that point.
         * @param maxAge Tile data maximum age in milliseconds, or -1 if the data does not expire.
         */
        void setMaxAge(long long maxAge);
        
        /**
         * Returns true if the tile should be replaced with parent tile.
         * @return True if the tile should be replaced with parent. False otherwise.
         */
        bool isReplaceWithParent() const;
        /**
         * Set the parent replacement flag.
         * @param flag True when the tile should be replaced with the parent, false otherwise.
         */
        void setReplaceWithParent(bool flag);
        
        /**
         * Returns tile data as binary data.
         * @return Tile data as binary data.
         */
        const std::shared_ptr<BinaryData>& getData() const;
        
    private:
        const std::shared_ptr<BinaryData> _data;
        std::shared_ptr<std::chrono::steady_clock::time_point> _expirationTime;
        bool _replaceWithParent;
        mutable std::mutex _mutex;
    };

}

#endif
