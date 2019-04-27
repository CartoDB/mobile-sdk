/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LICENSEMANAGER_H_
#define _CARTO_LICENSEMANAGER_H_

#include "components/DirectorPtr.h"

#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>

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
            
        static bool GetProductId(std::string& appParam, std::string& sdkProduct);
        static bool MatchProduct(const std::string& productTemplate, const std::string& product);
        static bool MatchAppId(const std::string& appIdTemplate, const std::string& appId);

        static std::unordered_map<std::string, std::string> DecodeLicense(const std::string& licenseKey);

        bool verifyLicenseParameters(const std::unordered_map<std::string, std::string>& parameters);

        std::string updateOnlineLicense();

        std::string _appId;
        std::unordered_map<std::string, std::string> _parameters;
        mutable std::mutex _mutex;

        mutable std::vector<std::thread> _updateThreads;
        mutable std::mutex _updateThreadsMutex;

        static const int LICENSESERVER_TIMEOUT = 5000; // in milliseconds
        static const std::string LICENSE_PREFIX;
        static const std::string LICENSE_SERVICE_URL;
        static const std::string PUBLIC_KEY;
        static const std::string PACKAGE_ENCRYPTION_KEY;
        static const std::string PRODUCT_VERSION;
    };
    
}

#endif
