#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "CartoPackageManager.h"
#include "utils/NetworkUtils.h"
#include "utils/PlatformUtils.h"
#include "utils/Log.h"

#include <regex>
#include <sstream>

namespace carto {

    CartoPackageManager::CartoPackageManager(const std::string& source, const std::string& dataFolder) :
        PackageManager(GetPackageListURL(source), dataFolder, SERVER_ENC_KEY, GetLocalEncKey()), _source(source)
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
    
    std::string CartoPackageManager::createPackageURL(const std::string& packageId, int version, const std::string& baseURL, bool downloaded) const {
        std::map<std::string, std::string> params;
        params["update"] = (downloaded ? "1" : "0");
        return NetworkUtils::BuildURLFromParameters(baseURL, params);
    }

    std::shared_ptr<PackageInfo> CartoPackageManager::getCustomPackage(const std::string& packageId, int version) const {
        static std::regex re("^bbox\\(([0-9-.eE]*,[0-9-.eE]*,[0-9-.eE]*,[0-9-.eE]*)\\)$");
        
        std::match_results<std::string::const_iterator> results;
        if (std::regex_match(packageId, results, re)) {
            std::string id = _source;
            std::string::size_type pos = _source.find(':');
            if (pos != std::string::npos) {
                id = _source.substr(pos + 1);
            }

            std::string baseURL = CUSTOM_BBOX_PACKAGE_URL + NetworkUtils::URLEncode(id) + "/1/extract";

            std::map<std::string, std::string> params;
            params["bbox"] = std::string(results[1].first, results[1].second);
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
                std::shared_ptr<PackageTileMask>(),
                std::shared_ptr<PackageMetaInfo>()
            );
            return packageInfo;
        }
        return std::shared_ptr<PackageInfo>();
    }

    const std::string CartoPackageManager::MAP_PACKAGE_LIST_URL = "http://api.nutiteq.com/v2/";

    const std::string CartoPackageManager::ROUTING_PACKAGE_LIST_URL = "http://api.nutiteq.com/v2/";

    const std::string CartoPackageManager::CUSTOM_BBOX_PACKAGE_URL = "http://api.nutiteq.com/v2/";
    
    const std::string CartoPackageManager::SERVER_ENC_KEY = "Hkz2jexmz9";

}

#endif
