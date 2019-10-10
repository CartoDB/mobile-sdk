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

    /**
     * An internal class for managing SDK licenses and services.
     */
    class LicenseManager {
    public:
        virtual ~LicenseManager();

        /**
         * Registers a license.
         * @param licenseKey The encoded license string.
         * @param listener The listener to call once the license has been verified and updated.
         * @return True if the license is valid.
         */
        bool registerLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener);

        /**
         * Returns the value of specified license parameter.
         * @param name The name of the parameter to return.
         * @param value The parameter value. It is set only if the method returns true.
         * @param wait If true, then SDK will wait until online license verification has completed and returns the value of the verified and updated parameter.
         * @return True if the specified parameter exists. False otherwise.
         */
        bool getParameter(const std::string& name, std::string& value, bool wait = false) const;

        /**
         * Returns the encryption key for offline packages.
         * @param key The encryption key. It is set only if the method returns true.
         * @return True if the offline package encryption key is available.
         */
        bool getPackageEncryptionKey(std::string& key) const;

        /**
         * Returns the singleton instance of the class.
         * @return The singleton instance of the class.
         */
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
