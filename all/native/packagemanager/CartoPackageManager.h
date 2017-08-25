/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOPACKAGEMANAGER_H_
#define _CARTO_CARTOPACKAGEMANAGER_H_

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "core/MapTile.h"
#include "core/MapBounds.h"
#include "packagemanager/PackageManager.h"

namespace carto {
    class Projection;
    
    /**
     * Offline map package manager that uses Carto online service for map package downloads.
     * After packages are downloaded, they can be used offline without any connection to the server.
     */
    class CartoPackageManager : public PackageManager {
    public:
        /**
         * Constructs a CartoPackageManager object, given application context and data folder.
         * The data folder must exist before creating a new package manager and it is assumed to be persistent.
         * Note: the package manager must be explicitly started using start() method!
         * @param source Name of the package source. Default: "nutiteq.osm"
         * @param dataFolder The folder where downloaded packages are kept. It must exist and must be writable.
         * @throws std::runtime_error If package manager fails to open or create package database.
         */
        CartoPackageManager(const std::string& source, const std::string& dataFolder);
        virtual ~CartoPackageManager();
        
    protected:
        static std::string GetPackageListURL(const std::string& source);
        static std::string GetServerEncKey();
        static std::string GetLocalEncKey();
        
        static bool CalculateBBoxTiles(const MapBounds& bounds, const std::shared_ptr<Projection>& proj, const MapTile& tile, std::vector<MapTile>& tiles);

        virtual std::string createPackageURL(const std::string& packageId, int version, const std::string& baseURL, bool downloaded) const;

        virtual std::shared_ptr<PackageInfo> getCustomPackage(const std::string& packageId, int version) const;
        
    private:
        struct PackageSource {
            std::string type;
            std::string id;

            PackageSource(const std::string& type, const std::string& id) : type(type), id(id) { }
        };

        static PackageSource ResolveSource(const std::string& source);

        static const std::string MAP_PACKAGE_LIST_URL;
        static const std::string ROUTING_PACKAGE_LIST_URL;
        static const std::string GEOCODING_PACKAGE_LIST_URL;

        static const std::string CUSTOM_MAP_BBOX_PACKAGE_URL;
        static const std::string CUSTOM_ROUTING_BBOX_PACKAGE_URL;
        static const std::string CUSTOM_GEOCODING_BBOX_PACKAGE_URL;

        static const int MAX_CUSTOM_BBOX_PACKAGE_TILES;
        static const int MAX_CUSTOM_BBOX_PACKAGE_TILE_ZOOM;
        static const int MAX_CUSTOM_BBOX_PACKAGE_TILEMASK_ZOOMLEVEL;
        static const int MAX_TILEMASK_LENGTH;
        
        std::string _source;
    };
}

#endif

#endif