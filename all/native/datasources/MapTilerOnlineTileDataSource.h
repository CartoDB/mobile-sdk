/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPTILERONLINETILEDATASOURCE_H_
#define _CARTO_MAPTILERONLINETILEDATASOURCE_H_

#include "datasources/TileDataSource.h"
#include "network/HTTPClient.h"

#include <random>
#include <vector>

namespace carto {
    
    /**
     * An online tile data source that connects to MapTiler Cloud tile server.
     * This data source should be used with vector tiles,
     * though by customizing service URL could be used with raster tiles also.
     * Be sure to read the Terms and Conditions of your MapTiler service provider to see if the
     * service is available for your application.
     * Note: this class is experimental and may change or even be removed in future SDK versions.
     */
    class MapTilerOnlineTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a MapTilerOnlineTileDataSource object.
         * @param key The access key (access token) to use registered with MapTiler.
         */
        explicit MapTilerOnlineTileDataSource(const std::string& key);
        virtual ~MapTilerOnlineTileDataSource();

        /**
         * Returns the custom backend service URL.
         * @return The custom backend service URL. If this is not defined, an empty string is returned.
         */
        std::string getCustomServiceURL() const;
        /**
         * Sets the custom backend service URL. 
         * The custom URL may contain tag "{key}" which will be substituted with the set access key.
         * @param serviceURL The custom backend service URL to use. If this is empty, then the default service is used.
         */
        void setCustomServiceURL(const std::string& serviceURL);

        /**
         * Returns the current timeout value.
         * @return The current timeout value in seconds. If negative, then default platform-specific timeout is used.
         */
        int getTimeout() const;
        /**
         * Sets the current timeout value.
         * @param timeout The new timeout value in seconds. If negative, then default platform-specific timeout is used.
         */
        void setTimeout(int timeout);

        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
        
    protected:
        std::string buildTileURL(const std::string& baseURL, const MapTile& tile) const;

        bool loadConfiguration();

        std::shared_ptr<TileData> loadOnlineTile(const std::string& url, const MapTile& mapTile);

        static const int DEFAULT_MAX_ZOOM;
        static const std::string MAPTILER_SERVICE_URL;

        const std::string _key;
        HTTPClient _httpClient;
        std::string _serviceURL;
        int _timeout;

        bool _tmsScheme;
        std::vector<std::string> _tileURLs;
        std::default_random_engine _randomGenerator;

        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
