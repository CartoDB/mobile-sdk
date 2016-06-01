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
        enum WatermarkType {
            EVALUATION_WATERMARK,
            EXPIRED_WATERMARK,
            CARTO_WATERMARK,
            CUSTOM_WATERMARK
        };

        virtual ~LicenseManager();

        bool registerLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener);

        std::string getUserKey() const;
        WatermarkType getWatermarkType() const;

        static LicenseManager& GetInstance();
        
    private:
        LicenseManager();
            
        static bool GetProductPlatformId(std::string& appParam, std::string& sdkProduct, std::string& platformId);
        static bool MatchProduct(const std::string& productTemplate, const std::string& product);
        static bool MatchAppId(const std::string& appIdTemplate, const std::string& appId);

        static std::unordered_map<std::string, std::string> DecodeLicense(const std::string& licenseKey);

        bool verifyLicenseParameters(const std::unordered_map<std::string, std::string>& parameters);

        std::string updateOnlineLicense();

        std::string _appId;
        std::string _userKey;
        WatermarkType _watermarkType;
        std::vector<std::thread> _updateThreads;
        mutable std::mutex _mutex;

        static const std::string LICENSE_PREFIX;
        static const std::string LICENSESERVER_URL;
        static const std::string PUBLIC_KEY;
        static const std::string PRODUCT_VERSION;
    };
    
}

#endif
