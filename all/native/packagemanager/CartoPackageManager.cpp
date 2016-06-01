#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "CartoPackageManager.h"
#include "components/LicenseManager.h"
#include "utils/NetworkUtils.h"
#include "utils/PlatformUtils.h"
#include "utils/Log.h"

#include <regex>
#include <sstream>

namespace carto {

    CartoPackageManager::CartoPackageManager(const std::string& source, const std::string& dataFolder) :
        PackageManager(GetPackageListUrl(source), dataFolder, SERVER_ENC_KEY, GetLocalEncKey()), _source(source)
    {
        if (!PlatformUtils::ExcludeFolderFromBackup(dataFolder)) {
            Log::Warn("CartoPackageManager: Failed to change package manager directory attributes");
        }
    }
    
    CartoPackageManager::~CartoPackageManager() {
    }
    
    std::string CartoPackageManager::GetPackageListUrl(const std::string& source) {
        std::string type = "map";
        std::string id = source;
        std::string::size_type pos = source.find(':');
        if (pos != std::string::npos) {
            type = source.substr(0, pos);
            id = source.substr(pos + 1);
        }

        std::string baseUrl;
        if (type == "map") {
            baseUrl = MAP_PACKAGE_LIST_URL + NetworkUtils::URLEncode(id) + "/packages";
        }
        else if (type == "routing") {
            baseUrl = ROUTING_PACKAGE_LIST_URL + NetworkUtils::URLEncode(id) + "/packages";
        }
        else {
            Log::Errorf("CartoPackageManager: Illegal package type: %s", type.c_str());
            return "";
        }
        std::map<std::string, std::string> params;
        params["user_key"] = LicenseManager::GetInstance().getUserKey();
        return NetworkUtils::BuildURLFromParameters(baseUrl, params);
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
    
    std::string CartoPackageManager::createPackageUrl(const std::string& packageId, int version, const std::string& baseUrl, bool downloaded) const {
        std::map<std::string, std::string> params;
        params["user_key"] = LicenseManager::GetInstance().getUserKey();
        params["update"] = (downloaded ? "1" : "0");
        return NetworkUtils::BuildURLFromParameters(baseUrl, params);
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
            std::map<std::string, std::string> params;
            params["bbox"] = std::string(results[1].first, results[1].second);
            std::string packageUrl = NetworkUtils::BuildURLFromParameters(CUSTOM_BBOX_PACKAGE_URL + NetworkUtils::URLEncode(id) + "/extract", params);

            auto packageInfo = std::make_shared<PackageInfo>(
                packageId,
                PackageType::PACKAGE_TYPE_MAP,
                version,
                0,
                packageUrl,
                std::shared_ptr<PackageTileMask>(),
                std::shared_ptr<PackageMetaInfo>()
            );
            return packageInfo;
        }
        return std::shared_ptr<PackageInfo>();
    }

    const std::string CartoPackageManager::MAP_PACKAGE_LIST_URL = "http://api.nutiteq.com/mappackages/v1/";

    const std::string CartoPackageManager::ROUTING_PACKAGE_LIST_URL = "http://api.nutiteq.com/mappackages/v1/";

    const std::string CartoPackageManager::CUSTOM_BBOX_PACKAGE_URL = "http://api.nutiteq.com/mappackages/v1/";
    
    const std::string CartoPackageManager::SERVER_ENC_KEY = "Hkz2jexmz9";

}

#endif
