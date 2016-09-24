#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "CartoPackageManager.h"
#include "components/Exceptions.h"
#include "components/LicenseManager.h"
#include "projections/EPSG3857.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/PlatformUtils.h"
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
        std::string type = "map";
        std::string id = source;
        std::string::size_type pos = source.find(':');
        if (pos != std::string::npos) {
            type = source.substr(0, pos);
            id = source.substr(pos + 1);
        }

        std::string baseURL;
        if (type == "map") {
            baseURL = MAP_PACKAGE_LIST_URL + NetworkUtils::URLEncode(id) + "/1/packages.json";
        }
        else if (type == "routing") {
            baseURL = ROUTING_PACKAGE_LIST_URL + NetworkUtils::URLEncode(id) + "/1/packages.json";
        }
        else {
            Log::Errorf("CartoPackageManager: Illegal package type: %s", type.c_str());
            return "";
        }

        std::map<std::string, std::string> params;
        params["appId"] = PlatformUtils::GetAppIdentifier();
        params["deviceId"] = PlatformUtils::GetDeviceId();
        params["platform"] = PlatformUtils::GetPlatformId();
        params["sdk_build"] = _CARTO_MOBILE_SDK_VERSION;
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

    bool CartoPackageManager::CalculateBBoxTiles(const MapBounds& bounds, const Projection& proj, const PackageTileMask::Tile& tile, std::vector<PackageTileMask::Tile>& tiles) {
        if (tile.zoom > MAX_CUSTOM_BBOX_PACKAGE_TILEMASK_ZOOM) {
            return true;
        }
        
        double tileWidth = proj.getBounds().getDelta().getX() / (1 << tile.zoom);
        double tileHeight = proj.getBounds().getDelta().getY() / (1 << tile.zoom);
        MapPos tileOrigin(proj.getBounds().getMin().getX() + tile.x * tileWidth, proj.getBounds().getMin().getY() + tile.y * tileHeight);
        MapBounds tileBounds(tileOrigin, MapPos(tileOrigin.getX() + tileWidth, tileOrigin.getY() + tileHeight));
        if (!tileBounds.intersects(bounds)) {
            return true;
        }

        if (tiles.size() >= MAX_CUSTOM_BBOX_PACKAGE_TILES) {
            return false;
        }
        tiles.push_back(tile);

        for (int dy = 0; dy < 2; dy++) {
            for (int dx = 0; dx < 2; dx++) {
                PackageTileMask::Tile subTile(tile.zoom + 1, tile.x * 2 + dx, tile.y * 2 + dy);
                if (!CalculateBBoxTiles(bounds, proj, subTile, tiles)) {
                    return false;
                }
            }
        }
        return true;
    }
    
    std::string CartoPackageManager::createPackageURL(const std::string& packageId, int version, const std::string& baseURL, bool downloaded) const {
        std::map<std::string, std::string> tagValues;
        std::string appToken;
        if (LicenseManager::GetInstance().getParameter("appToken", appToken)) {
            tagValues["key"] = appToken;
        }
 
        std::string url = GeneralUtils::ReplaceTags(baseURL, tagValues, "{", "}", true);

        std::map<std::string, std::string> params;
        params["update"] = (downloaded ? "1" : "0");
        return NetworkUtils::BuildURLFromParameters(url, params);
    }

    std::shared_ptr<PackageInfo> CartoPackageManager::getCustomPackage(const std::string& packageId, int version) const {
        static std::regex re("^bbox\\(\\s*([0-9-.eE]*)\\s*,\\s*([0-9-.eE]*)\\s*,\\s*([0-9-.eE]*)\\s*,\\s*([0-9-.eE]*)\\s*\\)$");
        
        std::match_results<std::string::const_iterator> results;
        if (std::regex_match(packageId, results, re)) {
            std::string id = _source;
            std::string::size_type pos = _source.find(':');
            if (pos != std::string::npos) {
                id = _source.substr(pos + 1);
            }

            EPSG3857 proj;
            MapBounds bounds;
            try {
                double minLon = boost::lexical_cast<double>(std::string(results[1].first, results[1].second));
                double minLat = boost::lexical_cast<double>(std::string(results[2].first, results[2].second));
                double maxLon = boost::lexical_cast<double>(std::string(results[3].first, results[3].second));
                double maxLat = boost::lexical_cast<double>(std::string(results[4].first, results[4].second));
                bounds = MapBounds(proj.fromLatLong(minLat, minLon), proj.fromLatLong(maxLat, maxLon));
            }
            catch (const boost::bad_lexical_cast&) {
                Log::Error("CartoPackageManager: Illegal bounding box coordinates");
                return std::shared_ptr<PackageInfo>();
            }

            std::vector<PackageTileMask::Tile> tiles;
            if (!CalculateBBoxTiles(bounds, proj, PackageTileMask::Tile(0, 0, 0), tiles)) {
                Log::Error("CartoPackageManager: Too many tiles in custom package");
                return std::shared_ptr<PackageInfo>();
            }

            auto tileMask = std::make_shared<PackageTileMask>(tiles);
            std::string baseURL = CUSTOM_BBOX_PACKAGE_URL + NetworkUtils::URLEncode(id) + "/custom/" + NetworkUtils::URLEncode(tileMask->getStringValue()) + ".mbtiles";

            std::map<std::string, std::string> params;
            params["appId"] = PlatformUtils::GetAppIdentifier();
            params["deviceId"] = PlatformUtils::GetDeviceId();
            params["platform"] = PlatformUtils::GetPlatformId();
            params["sdk_build"] = _CARTO_MOBILE_SDK_VERSION;
            std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);

            auto packageInfo = std::make_shared<PackageInfo>(
                packageId,
                PackageType::PACKAGE_TYPE_MAP,
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

    const std::string CartoPackageManager::MAP_PACKAGE_LIST_URL = "http://api-staging.nutiteq.com/v2/";

    const std::string CartoPackageManager::ROUTING_PACKAGE_LIST_URL = "http://api-staging.nutiteq.com/v2/";

    const std::string CartoPackageManager::CUSTOM_BBOX_PACKAGE_URL = "http://api-staging.nutiteq.com/v2/";
    
}

#endif
