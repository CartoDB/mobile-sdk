#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "CartoPackageManager.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "components/LicenseManager.h"
#include "projections/EPSG3857.h"
#include "packagemanager/handlers/PackageHandlerFactory.h"
#include "vectortiles/utils/CartoAssetPackageUpdater.h"
#include "utils/MemoryAssetPackage.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/PlatformUtils.h"
#include "utils/TileUtils.h"
#include "utils/Log.h"

#include <regex>
#include <sstream>

#include <sqlite3pp.h>

#include <boost/lexical_cast.hpp>

namespace carto {

    CartoPackageManager::CartoPackageManager(const std::string& source, const std::string& dataFolder) :
        PackageManager(GetPackageListURL(source), dataFolder, GetServerEncKey(), GetLocalEncKey()),
        _source(source),
        _styleDbMutex()
    {
        if (!PlatformUtils::ExcludeFolderFromBackup(dataFolder)) {
            Log::Warn("CartoPackageManager: Failed to change package manager directory attributes");
        }
    }

    CartoPackageManager::~CartoPackageManager() {
    }

    std::shared_ptr<AssetPackage> CartoPackageManager::getStyleAssetPackage(CartoBaseMapStyle::CartoBaseMapStyle style) const {
        return getStyleAssetPackage(CartoVectorTileLayer::GetStyleName(style));
    }

    bool CartoPackageManager::startStyleDownload(CartoBaseMapStyle::CartoBaseMapStyle style) {
        return PackageManager::startStyleDownload(CartoVectorTileLayer::GetStyleName(style));
    }

    std::string CartoPackageManager::GetPackageListURL(const std::string& source) {
        PackageSource packageSource = ResolveSource(source);

        std::string baseURL;
        switch (packageSource.type) {
        case PackageType::PACKAGE_TYPE_MAP:
            baseURL = NetworkUtils::CreateServiceURL(MAP_PACKAGE_LIST_TEMPLATE, packageSource.id);
            break;
        case PackageType::PACKAGE_TYPE_ROUTING:
        case PackageType::PACKAGE_TYPE_VALHALLA_ROUTING:
            baseURL = NetworkUtils::CreateServiceURL(ROUTING_PACKAGE_LIST_TEMPLATE, packageSource.id);
            break;
        case PackageType::PACKAGE_TYPE_GEOCODING:
            baseURL = NetworkUtils::CreateServiceURL(GEOCODING_PACKAGE_LIST_TEMPLATE, packageSource.id);
            break;
        default:
            Log::Errorf("CartoPackageManager: Failed to resolve source: %s", source.c_str());
            return std::string();
        }

        std::map<std::string, std::string> params;
        params["deviceId"] = PlatformUtils::GetDeviceId();
        params["platform"] = PlatformUtils::GetPlatformId();
        params["sdk_build"] = PlatformUtils::GetSDKVersion();
        std::string appToken;
        if (LicenseManager::GetInstance().getParameter("appToken", appToken, false)) {
            params["appToken"] = appToken;
        } else {
            Log::Error("CartoPackageManager::GetPackageListURL: appToken not available. License issue?");
            return std::string();
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
            std::string baseURLTemplate;
            switch (packageSource.type) {
            case PackageType::PACKAGE_TYPE_MAP:
                baseURLTemplate = NetworkUtils::CreateServiceURL(CUSTOM_MAP_BBOX_PACKAGE_TEMPLATE, packageSource.id);
                break;
            case PackageType::PACKAGE_TYPE_ROUTING:
            case PackageType::PACKAGE_TYPE_VALHALLA_ROUTING:
                baseURLTemplate = NetworkUtils::CreateServiceURL(CUSTOM_ROUTING_BBOX_PACKAGE_TEMPLATE, packageSource.id);
                packageSource.type = PackageType::PACKAGE_TYPE_VALHALLA_ROUTING; // kind of hack, we currently only support Valhalla custom extracts
                break;
            case PackageType::PACKAGE_TYPE_GEOCODING:
                baseURLTemplate = NetworkUtils::CreateServiceURL(CUSTOM_GEOCODING_BBOX_PACKAGE_TEMPLATE, packageSource.id);
                break;
            default:
                Log::Errorf("CartoPackageManager: Failed to resolve source: %s", _source.c_str());
                return std::shared_ptr<PackageInfo>();
            }

            std::map<std::string, std::string> tagValues = { { "tilemask", NetworkUtils::URLEncode(tileMask->getURLSafeStringValue()) } };
            std::string baseURL = GeneralUtils::ReplaceTags(baseURLTemplate, tagValues, "{", "}", true);

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
                packageSource.type,
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

    bool CartoPackageManager::updateStyle(const std::string& styleName) {
        std::shared_ptr<AssetPackage> currentAssetPackage = getStyleAssetPackage(styleName);
        std::string schema = getSchema();
        if (schema.empty()) {
            schema = _source + "/v2"; // default schema, if missing
        }

        std::shared_ptr<MemoryAssetPackage> newAssetPackage;
        try {
            CartoAssetPackageUpdater updater(schema, styleName);
            newAssetPackage = updater.update(currentAssetPackage);
        }
        catch (const std::exception& ex) {
            Log::Errorf("CartoPackageManager::updateStyle: Error while updating style: %s", ex.what());
            return false;
        }

        if (newAssetPackage) {
            std::lock_guard<std::recursive_mutex> lock(_styleDbMutex);
            std::shared_ptr<sqlite3pp::database> styleDb = createStyleDb(styleName);
            sqlite3pp::transaction xct(*styleDb);
            for (const std::string& fileName : newAssetPackage->getLocalAssetNames()) {
                std::shared_ptr<BinaryData> data = newAssetPackage->loadAsset(fileName);
                sqlite3pp::command delCmd(*styleDb, "DELETE FROM files WHERE filename=:fileName");
                delCmd.bind(":fileName", fileName.c_str());
                delCmd.execute();
                if (data) {
                    sqlite3pp::command insCmd(*styleDb, "INSERT INTO files (filename, contents) VALUES(:fileName, :contents)");
                    insCmd.bind(":fileName", fileName.c_str());
                    insCmd.bind(":contents", data->data(), static_cast<int>(data->size()));
                    insCmd.execute();
                }
            }
            xct.commit();
        }

        return newAssetPackage && !newAssetPackage->getLocalAssetNames().empty();
    }

    std::shared_ptr<sqlite3pp::database> CartoPackageManager::createStyleDb(const std::string& styleName) const {
        std::string dbFileName = createLocalFilePath("style_" + styleName + "_files.sqlite");
        auto db = std::make_shared<sqlite3pp::database>(dbFileName.c_str());
        db->execute("PRAGMA encoding='UTF-8'");

        db->execute(R"SQL(
                CREATE TABLE IF NOT EXISTS files (
                    filename TEXT NOT NULL PRIMARY KEY,
                    contents BLOB NULL
                ))SQL");
        return db;
    }

    std::shared_ptr<AssetPackage> CartoPackageManager::getStyleAssetPackage(const std::string& styleName) const {
        std::shared_ptr<AssetPackage> styleAssetPackage = CartoVectorTileLayer::CreateStyleAssetPackage();

        std::lock_guard<std::recursive_mutex> lock(_styleDbMutex);

        std::shared_ptr<sqlite3pp::database> styleDb = createStyleDb(styleName);

        sqlite3pp::query query(*styleDb, "SELECT filename, contents FROM files");
        std::map<std::string, std::shared_ptr<BinaryData> > updatedAssets;
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::string fileName = qit->get<const char*>(0);
            std::shared_ptr<BinaryData> contents;
            if (qit->get<const void*>(1)) {
                contents = std::make_shared<BinaryData>(static_cast<const unsigned char*>(qit->get<const void*>(1)), qit->column_bytes(1));
            }
            updatedAssets[fileName] = contents;
        }
        return std::make_shared<MemoryAssetPackage>(updatedAssets, styleAssetPackage);
    }

    CartoPackageManager::PackageSource CartoPackageManager::ResolveSource(const std::string& source) {
        if (source.find("://") != std::string::npos) {
            return PackageSource(PackageHandlerFactory::DetectPackageType(source), source);
        }

        std::string::size_type pos = source.find(':');
        if (pos != std::string::npos) {
            std::string type = source.substr(0, pos);
            std::string realSource = source.substr(pos + 1);
            if (type == "map") {
                return PackageSource(PackageType::PACKAGE_TYPE_MAP, realSource);
            } else if (type == "routing") {
                return PackageSource(PackageType::PACKAGE_TYPE_ROUTING, realSource); // Note: we do not differentiate between Valhalla/legacy routing here
            } else if (type == "geocoding") {
                return PackageSource(PackageType::PACKAGE_TYPE_GEOCODING, realSource);
            }
            Log::Errorf("CartoPackageManager::ResolveSource: Unsupported type: %s", type.c_str());
        }
        return PackageSource(PackageType::PACKAGE_TYPE_MAP, source);
    }

    const std::string CartoPackageManager::MAP_PACKAGE_LIST_TEMPLATE = "https://api.nutiteq.com/mappackages/v2/{source}/2/packages.json";

    const std::string CartoPackageManager::ROUTING_PACKAGE_LIST_TEMPLATE = "https://api.nutiteq.com/routepackages/v2/{source}/2/packages.json";

    const std::string CartoPackageManager::GEOCODING_PACKAGE_LIST_TEMPLATE = "https://api.nutiteq.com/geocodepackages/v2/{source}/1/packages.json";

    const std::string CartoPackageManager::CUSTOM_MAP_BBOX_PACKAGE_TEMPLATE = "https://api.nutiteq.com/maparea/v2/{source}/1/{tilemask}.mbtiles";

    const std::string CartoPackageManager::CUSTOM_ROUTING_BBOX_PACKAGE_TEMPLATE = "https://api.nutiteq.com/routearea/v2/{source}/2/{tilemask}.vtiles";

    const std::string CartoPackageManager::CUSTOM_GEOCODING_BBOX_PACKAGE_TEMPLATE = "https://api.nutiteq.com/geocodearea/v2/{source}/1/{tilemask}.nutigeodb";

    const unsigned int CartoPackageManager::MAX_CUSTOM_BBOX_PACKAGE_TILES = 250000;

    const int CartoPackageManager::MAX_CUSTOM_BBOX_PACKAGE_TILE_ZOOM = 14;

    const int CartoPackageManager::MAX_CUSTOM_BBOX_PACKAGE_TILEMASK_ZOOMLEVEL = 12;

    const unsigned int CartoPackageManager::MAX_TILEMASK_LENGTH = 128;

}

#endif
