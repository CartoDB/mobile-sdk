/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TILEDOWNLOADLISTENER_H_
#define _CARTO_TILEDOWNLOADLISTENER_H_

#include "core/MapTile.h"

#include <memory>

namespace carto {
    
    /**
     * Listener for tile downloader.
     */
    class TileDownloadListener {
    public:
        virtual ~TileDownloadListener() { }
    
        /**
         * Listener method that is called before download has actually started to report the total tile count.
         * @param tileCount The number of tiles that will be downloaded (if not already in the cache).
         */
        virtual void onDownloadStarting(int tileCount) { }
        /**
         * Listener method that is called to report about download progress.
         * @param progress The progress of the download, from 0 to 100.
         */
        virtual void onDownloadProgress(float progress) { }
        /**
         * Listener method that is called when downloading has finished.
         */
        virtual void onDownloadCompleted() { }
        /**
         * Listener method that is called when a tile download fails.
         * @param tile The tile that could not be downloaded.
         */
        virtual void onDownloadFailed(const MapTile& tile) { }
    };
    
}

#endif
