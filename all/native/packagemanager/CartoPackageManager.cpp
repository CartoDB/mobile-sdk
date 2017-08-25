#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "CartoPackageManager.h"
#include "components/Exceptions.h"
#include "components/LicenseManager.h"
#include "projections/EPSG3857.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/PlatformUtils.h"
#include "utils/TileUtils.h"
#include "utils/Log.h"

#include <regex>
#include <sstream>

#include <boost/lexical_cast.hpp>

namespace carto {

    CartoPackageManager::CartoPackageManager(const std::string& source, const std::string& dataFolder) :
        PackageManager(GetPackageListURL(source), dataFolder, GetServerEncKey(), GetLocalEncKey()), _source(source)
    {
        if (!PlatformUtils::ExcludeFolderFromBackup(dataFolder)) {
            Log::Warn("CartoPackageManager: Failed to change package manager directory attributes");
        }
    }

    CartoPackageManager::~CartoPackageManager() {
    }

    std::string CartoPackageManager::GetPackageListURL(const std::string& source) {
        PackageSource packageSource = ResolveSource(source);

        std::string baseURL;
        if (packageSource.type == "map") {
            baseURL = MAP_PACKAGE_LIST_URL + NetworkUtils::URLEncode(packageSource.id) + "/2/packages.json";
        }
        else if (packageSource.type == "routing") {
            baseURL = ROUTING_PACKAGE_LIST_URL + NetworkUtils::URLEncode(packageSource.id) + "/1/packages.json";
        }
        else if (packageSource.type == "geocoding") {
            baseURL = GEOCODING_PACKAGE_LIST_URL + NetworkUtils::URLEncode(packageSource.id) + "/1/packages.json";
        }
        else {
            Log::Errorf("CartoPackageManager: Illegal package type: %s", packageSource.type.c_str());
            return "";
        }

        std::map<std::string, std::string> params;
        params["deviceId"] = PlatformUtils::GetDeviceId();
        params["platform"] = PlatformUtils::GetPlatformId();
        params["sdk_build"] = PlatformUtils::GetSDKVersion();
        std::string appToken;
        if (LicenseManager::GetInstance().getParameter("appToken", appToken, false)) {
            params["appToken"] = appToken;
        }
        return NetworkUtils::BuildURLFromParameters(baseURL, params);
    }

    std::string CartoPackageManager::GetServerEncKey() {
        std::string encKey;
        if (!LicenseManager::GetInstance().getPackageEncryptionKey(encKey)) {
            throw GenericException("Offline packages not supported");
        }
        return encKey;
    }

    std::string CartoPackageManager::GetLocalEncKey() {
        std::string encKey = PlatformUtils::GetAppDeviceId();
        if (encKey.empty()) {
            Log::Error("CartoPackageManager: RegisterLicense not called, using random key for package encryption!");
            std::stringstream ss;
            ss << static_cast<unsigned int>(time(NULL));
            encKey = ss.str();
        }
        return encKey;
    }

    bool CartoPackageManager::CalculateBBoxTiles(const MapBounds& bounds, const std::shared_ptr<Projection>& proj, const MapTile& tile, std::vector<MapTile>& tiles) {
        if (tile.getZoom() > MAX_CUSTOM_BBOX_PACKAGE_TILE_ZOOM) {
            return true;
        }
        
        if (!bounds.intersects(TileUtils::CalculateMapTileBounds(tile, proj))) {
            return true;
        }

        if (tiles.size() >= MAX_CUSTOM_BBOX_PACKAGE_TILES) {
            return false;
        }
        tiles.push_back(tile);

        for (int i = 0; i < 4; i++) {
            MapTile subTile = tile.getChild(i);
            if (!CalculateBBoxTiles(bounds, proj, subTile, tiles)) {
                return false;
            }
        }
        return true;
    }
    
    std::string CartoPackageManager::createPackageURL(const std::string& packageId, int version, const std::string& baseURL, bool downloaded) const {
        std::string appToken;
        if (!LicenseManager::GetInstance().getParameter("appToken", appToken, false)) {
            return std::string(); // invalid URL
        }
 
        std::map<std::string, std::string> tagValues;
        tagValues["key"] = appToken;
        std::string url = GeneralUtils::ReplaceTags(baseURL, tagValues, "{", "}", true);

        std::map<std::string, std::string> params;
        params["update"] = (downloaded ? "1" : "0");
        return NetworkUtils::BuildURLFromParameters(url, params);
    }

    std::shared_ptr<PackageInfo> CartoPackageManager::getCustomPackage(const std::string& packageId, int version) const {
        static std::regex re("^bbox\\(\\s*([0-9-.eE]*)\\s*,\\s*([0-9-.eE]*)\\s*,\\s*([0-9-.eE]*)\\s*,\\s*([0-9-.eE]*)\\s*\\)$");
        
        std::match_results<std::string::const_iterator> results;
        if (std::regex_match(packageId, results, re)) {
            PackageSource packageSource = ResolveSource(_source);

            auto proj = std::make_shared<EPSG3857>();
            MapBounds bounds;
            try {
                double minLon = boost::lexical_cast<double>(std::string(results[1].first, results[1].second));
                double minLat = boost::lexical_cast<double>(std::string(results[2].first, results[2].second));
                double maxLon = boost::lexical_cast<double>(std::string(results[3].first, results[3].second));
                double maxLat = boost::lexical_cast<double>(std::string(results[4].first, results[4].second));
                if (minLon >= maxLon || minLat >= maxLat) {
                    Log::Warn("CartoPackageManager: Empty bounding box");
                    return std::shared_ptr<PackageInfo>();
                }
                bounds = MapBounds(proj->fromLatLong(minLat, minLon), proj->fromLatLong(maxLat, maxLon));
            }
            catch (const boost::bad_lexical_cast&) {
                Log::Error("CartoPackageManager: Illegal bounding box coordinates");
                return std::shared_ptr<PackageInfo>();
            }

            // Build explicit tile list
            std::vector<MapTile> tiles;
            if (!CalculateBBoxTiles(bounds, proj, MapTile(0, 0, 0, 0), tiles)) {
                Log::Error("CartoPackageManager: Too many tiles in custom package");
                return std::shared_ptr<PackageInfo>();
            }

            // Build tilemask. If the tilemask string is too long, use higher zoom levels
            std::shared_ptr<PackageTileMask> tileMask;
            for (int zoom = MAX_CUSTOM_BBOX_PACKAGE_TILEMASK_ZOOMLEVEL; zoom >= 0; zoom--) {
                tileMask = std::make_shared<PackageTileMask>(tiles, zoom);
                if (tileMask->getURLSafeStringValue().size() <= MAX_TILEMASK_LENGTH) {
                    break;
                }
            }

            // Configure service URL
            std::string baseURL;
            PackageType::PackageType packageType = PackageType::PACKAGE_TYPE_MAP;
            if (packageSource.type == "map") {
                baseURL = CUSTOM_MAP_BBOX_PACKAGE_URL + NetworkUtils::URLEncode(packageSource.id) + "/1/" + NetworkUtils::URLEncode(tileMask->getURLSafeStringValue()) + ".mbtiles";
                packageType = PackageType::PACKAGE_TYPE_MAP;
            }
            else if (packageSource.type == "routing") {
                baseURL = CUSTOM_ROUTING_BBOX_PACKAGE_URL + NetworkUtils::URLEncode(packageSource.id) + "/1/" + NetworkUtils::URLEncode(tileMask->getURLSafeStringValue()) + ".vtiles";
                packageType = PackageType::PACKAGE_TYPE_VALHALLA_ROUTING;
            }
            else if (packageSource.type == "geocoding") {
                baseURL = CUSTOM_GEOCODING_BBOX_PACKAGE_URL + NetworkUtils::URLEncode(packageSource.id) + "/1/" + NetworkUtils::URLEncode(tileMask->getURLSafeStringValue()) + ".nutigeodb";
                packageType = PackageType::PACKAGE_TYPE_GEOCODING;
            }
            else {
                Log::Errorf("CartoPackageManager: Illegal package type: %s", packageSource.type.c_str());
                return std::shared_ptr<PackageInfo>();
            }

            std::map<std::string, std::string> params;
            params["deviceId"] = PlatformUtils::GetDeviceId();
            params["platform"] = PlatformUtils::GetPlatformId();
            params["sdk_build"] = PlatformUtils::GetSDKVersion();
            std::string appToken;
            if (LicenseManager::GetInstance().getParameter("appToken", appToken, false)) {
                params["appToken"] = appToken;
            }
            std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);

            // Create package info
            auto packageInfo = std::make_shared<PackageInfo>(
                packageId,
                packageType,
                version,
                0,
                url,
                tileMask,
                std::shared_ptr<PackageMetaInfo>()
                );
            return packageInfo;
        }
        return std::shared_ptr<PackageInfo>();
    }

    CartoPackageManager::PackageSource CartoPackageManager::ResolveSource(const std::string& source) {
        PackageSource packageSource("map", source);
        std::string::size_type pos = source.find(':');
        if (pos != std::string::npos) {
            packageSource.type = source.substr(0, pos);
            packageSource.id = source.substr(pos + 1);
        }
        return packageSource;
    }

    const std::string CartoPackageManager::MAP_PACKAGE_LIST_URL = "http://api.nutiteq.com/mappackages/v2/";

    const std::string CartoPackageManager::ROUTING_PACKAGE_LIST_URL = "http://api.nutiteq.com/routepackages/v2/";

    const std::string CartoPackageManager::GEOCODING_PACKAGE_LIST_URL = "http://api.nutiteq.com/geocodepackages/v2/";

    const std::string CartoPackageManager::CUSTOM_MAP_BBOX_PACKAGE_URL = "http://api.nutiteq.com/maparea/v2/";

    const std::string CartoPackageManager::CUSTOM_ROUTING_BBOX_PACKAGE_URL = "http://api.nutiteq.com/routearea/v2/";

    const std::string CartoPackageManager::CUSTOM_GEOCODING_BBOX_PACKAGE_URL = "http://api.nutiteq.com/geocodearea/v2/";

    const int CartoPackageManager::MAX_CUSTOM_BBOX_PACKAGE_TILES = 250000;

    const int CartoPackageManager::MAX_CUSTOM_BBOX_PACKAGE_TILE_ZOOM = 14;

    const int CartoPackageManager::MAX_CUSTOM_BBOX_PACKAGE_TILEMASK_ZOOMLEVEL = 12;

    const int CartoPackageManager::MAX_TILEMASK_LENGTH = 128;

}

#endif