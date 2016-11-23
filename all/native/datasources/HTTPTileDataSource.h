/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HTTPTILEDATASOURCE_H_
#define _CARTO_HTTPTILEDATASOURCE_H_

#include "datasources/TileDataSource.h"
#include "network/HTTPClient.h"

#include <random>
#include <string>
#include <map>
#include <vector>
#include <mutex>

namespace carto {

    /**
     * A tile data source that loads tiles using a HTTP connection.
     * The requests are generated using a template scheme, where tags in the baseURL string are replaced with actual values.
     * The following tags are supported: s, z, zoom, x, y, xflipped, yflipped, quadkey, frame.
     *
     * For example, if baseURL = "http://tile.openstreetmap.org/{zoom}/{x}/{y}.png" and the requested tile has zoom = 2,
     * x = 1 and y = 3, then the tile will be loaded from the following URL: "http://tile.openstreetmap.org/2/1/3.png".
     */
    class HTTPTileDataSource : public TileDataSource {
    public:
        /**
         * Constructs a HTTPTileDataSource object.
         * @param minZoom The minimum zoom level supported by this data source.
         * @param maxZoom The maximum zoom level supported by this data source.
         * @param baseURL The base URL containing tags (for example, "http://tile.openstreetmap.org/{zoom}/{x}/{y}.png").
         */
        HTTPTileDataSource(int minZoom, int maxZoom, const std::string& baseURL);
        virtual ~HTTPTileDataSource();
        
        /**
         * Returns the base URL template containing tags.
         * @return The base URL template.
         */
        std::string getBaseURL() const;
        /**
         * Sets the base URL for the data source.
         * @param baseURL The base URL containing tags (for example, "http://tile.openstreetmap.org/{zoom}/{x}/{y}.png").
         */
        void setBaseURL(const std::string& baseURL);

        /**
         * Returns the subdomains for {s} tag. The default is ["a", "b", "c", "d"].
         * @return The list of subdomains.
         */
        std::vector<std::string> getSubdomains() const;
        /**
         * Sets the subdomains for {s} tag.
         * @param subdomains The list of subdomains to use.
         */
        void setSubdomains(const std::vector<std::string>& subdomains);

        /**
         * Returns true/false based whether the TMS tiling scheme is used.
         * @return True if TMS tiling scheme is used. False if XYZ scheme is used.
         */
        bool isTMSScheme() const;
        /**
         * Enables/disables the TMS tiling scheme. In TMS scheme y coordinate of the tile is flipped. The default is disabled.
         * @param tmsScheme True is TMS tiling scheme should be used. False is XYZ should be used.
         */
        void setTMSScheme(bool tmsScheme);

        /**
         * Returns true/false based on whether the max-age header check is used.
         * If this is enabled, SDK will automatically refresh the tiles when tiles have expired.
         * @return True if max-age header check is used. False otherwise.
         */
        bool isMaxAgeHeaderCheck() const;
        /**
         * Enables/disables the max-age header check.
         * If this is enabled, SDK will automatically refresh the tiles when tiles have expired. The default is disabled.
         * @param maxAgeCheck True if the check should be enabled, false otherwise.
         */
        void setMaxAgeHeaderCheck(bool maxAgeCheck);
        
        /**
         * Returns the current set of HTTP headers used. Initially this set is empty and can be changed with setHTTPHeaders.
         * @returns The current set of custom HTTP headers.
         */
        std::map<std::string, std::string> getHTTPHeaders() const;		
        /**
         * Sets HTTP headers for all requests. Calling this method will invalidate the datasource and
         * all layers using this data source will be refreshed.
         * @param headers A map of HTTP headers that will be used in subsequent requests.
         */
        void setHTTPHeaders(const std::map<std::string, std::string>& headers);
    
        virtual std::shared_ptr<TileData> loadTile(const MapTile& mapTile);
    
    protected:
        virtual std::string buildTileURL(const std::string& baseURL, const MapTile& tile) const;
    
        std::string _baseURL;
        std::vector<std::string> _subdomains;
        bool _tmsScheme;
        bool _maxAgeHeaderCheck;
        std::map<std::string, std::string> _headers;
        HTTPClient _httpClient;
        mutable std::default_random_engine _randomGenerator;
        mutable std::mutex _mutex;
    };
    
}

#endif
