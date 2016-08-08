#ifndef _CARTO_LICENSEMANAGER_SUPPORT

#include "LicenseManager.h"
#include "components/LicenseManagerListener.h"
#include "utils/Log.h"

namespace carto {

    bool LicenseManager::registerLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener) {
        Log::Info("LicenseManager::registerLicense: Public build, accepting any license!");
        if (listener) {
            listener->onLicenseUpdated(licenseKey);
        }
        return true;
    }

    bool LicenseManager::getParameter(const std::string& name, std::string& value) const {
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
