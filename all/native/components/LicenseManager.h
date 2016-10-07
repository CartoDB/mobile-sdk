/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LICENSEMANAGER_H_
#define _CARTO_LICENSEMANAGER_H_

#ifndef _CARTO_LICENSEMANAGER_SUPPORT

#include <string>
#include <memory>
#include <unordered_map>

namespace carto {
    class LicenseManagerListener;

    class LicenseManager {
    public:
        virtual ~LicenseManager();

        bool registerLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener);

        bool getParameter(const std::string& name, std::string& value, bool wait = false) const;

        bool getPackageEncryptionKey(std::string& key) const;

        static LicenseManager& GetInstance();
        
    private:
        LicenseManager();
    };
    
}

#endif

#endif
