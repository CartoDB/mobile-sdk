#ifndef _CARTO_LICENSEMANAGER_SUPPORT

#include "LicenseManager.h"
#include "components/LicenseManagerListener.h"
#include "utils/Log.h"

namespace carto {

    bool LicenseManager::registerLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener) {
        Log::Info("LicenseManager::registerLicense: Public build, accepting any license!");
        return true;
    }

    bool LicenseManager::getParameter(const std::string& name, std::string& value) const {
        if (name == "watermark") {
            return "carto";
        }
        if (name == "appToken") {
            Log::Error("LicenseManager::getParameter: Public build, appToken not available!");
        }
        return false;
    }

    bool LicenseManager::getPackageEncryptionKey(std::string& key) const {
        Log::Error("LicenseManager::getPackageEncryptionKey: Public build, offline packages not available!");
        return false;
    }
    
    LicenseManager& LicenseManager::GetInstance() {
        static LicenseManager instance;
        return instance;
    }

    LicenseManager::LicenseManager() {
    }

    LicenseManager::~LicenseManager() {
    }

}

#endif
