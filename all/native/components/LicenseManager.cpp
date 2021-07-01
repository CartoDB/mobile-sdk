#include "LicenseManager.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "components/LicenseManagerListener.h"
#include "network/HTTPClient.h"
#include "utils/PlatformUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <tuple>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <picojson/picojson.h>

#include <botan/botan_all.h>

namespace carto {

    bool LicenseManager::registerLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener) {
        bool result = false;
        std::unordered_map<std::string, std::string> parameters;
        try {
            std::lock_guard<std::mutex> lock(_mutex);
            parameters = DecodeLicense(licenseKey);
            result = verifyLicenseParameters(parameters);
            if (!result) {
                _parameters["watermark"] = "expired";
            }
        }
        catch (const std::exception& ex) {
            std::lock_guard<std::mutex> lock(_mutex);
            _parameters["watermark"] = "expired";
            Log::Errorf("LicenseManager::registerLicense: Invalid license: %s", ex.what());
            return false;
        }
        
        auto it = parameters.find("onlineLicense");
        if (it != parameters.end()) {
            if (!it->second.empty() && it->second != "0") {
                std::lock_guard<std::mutex> updateThreadsLock(_updateThreadsMutex);

                // Keep the listener reference as DirectorPtr as we need it before then method returns
                DirectorPtr<LicenseManagerListener> listenerPtr(listener);
                std::thread updateThread([listenerPtr, parameters, this]() {
                    std::string updatedLicenseKey = updateOnlineLicense();
                    
                    // Call listener with updated license string
                    if (!updatedLicenseKey.empty()) {
                        if (listenerPtr) {
                            listenerPtr->onLicenseUpdated(updatedLicenseKey);
                        }
                    }
                });

                _updateThreads.push_back(std::move(updateThread));
            }
        }

        return result;
    }

    bool LicenseManager::getParameter(const std::string& name, std::string& value, bool wait) const {
        if (wait) {
            // Wait until all update threads have finished
            std::lock_guard<std::mutex> lock(_updateThreadsMutex);
            while (!_updateThreads.empty()) {
                _updateThreads.front().join();
                _updateThreads.erase(_updateThreads.begin());
            }
        }

        // Try to get the parameter
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _parameters.find(name);
        if (it != _parameters.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    bool LicenseManager::getPackageEncryptionKey(std::string& key) const {
        key = PACKAGE_ENCRYPTION_KEY;
        return true;
    }
    
    LicenseManager& LicenseManager::GetInstance() {
        static LicenseManager instance;
        return instance;
    }

    LicenseManager::LicenseManager() :
        _appId(),
        _parameters(),
        _mutex(),
        _updateThreads(),
        _updateThreadsMutex()
    {
        _parameters["watermark"] = "development";
    }

    LicenseManager::~LicenseManager() {
        for (std::thread& updateThread : _updateThreads) {
            updateThread.join();
        }
    }

    bool LicenseManager::GetProductId(std::string& appParam, std::string& sdkProduct) {
        switch (PlatformUtils::GetPlatformType()) {
        case PlatformType::PLATFORM_TYPE_ANDROID:
            appParam = "packageName";
            sdkProduct = "sdk-android-";
            break;
        case PlatformType::PLATFORM_TYPE_IOS:
            appParam = "bundleIdentifier";
            sdkProduct = "sdk-ios-";
            break;
        case PlatformType::PLATFORM_TYPE_XAMARIN_ANDROID:
            appParam = "packageName";
            sdkProduct = "sdk-xamarin-android-";
            break;
        case PlatformType::PLATFORM_TYPE_XAMARIN_IOS:
            appParam = "bundleIdentifier";
            sdkProduct = "sdk-xamarin-ios-";
            break;
        case PlatformType::PLATFORM_TYPE_WINDOWS_PHONE:
            appParam = "productId";
            sdkProduct = "sdk-winphone-";
            break;
        default:
            return false;
        }
        sdkProduct += PRODUCT_VERSION;
        return true;
    }
    
    bool LicenseManager::MatchProduct(const std::string& productTemplate, const std::string& product) {
        std::vector<std::string> productREs;
        boost::split(productREs, productTemplate, boost::is_any_of(","));
        for (std::string productRE : productREs) {
            productRE = boost::replace_all_copy(boost::replace_all_copy(productRE, ".", "[.]"), "*", ".*");
            if (std::regex_match(product, std::regex(productRE))) {
                return true;
            }
        }
        return false;
    }

    bool LicenseManager::MatchAppId(const std::string& appIdTemplate, const std::string& appId) {
        std::vector<std::string> appIdREs;
        boost::split(appIdREs, appIdTemplate, boost::is_any_of(","));
        for (std::string appIdRE : appIdREs) {
            appIdRE = boost::replace_all_copy(boost::replace_all_copy(appIdRE, ".", "[.]"), "*", ".*");
            if (std::regex_match(appId, std::regex(appIdRE))) {
                return true;
            }
        }
        return false;
    }

    std::unordered_map<std::string, std::string> LicenseManager::DecodeLicense(const std::string& licenseKey) {
        // Unsalt and decode the license string
        if (licenseKey.substr(0, LICENSE_PREFIX.size()) != LICENSE_PREFIX) {
            throw ParseException("Invalid license prefix");
        }
        Botan::secure_vector<std::uint8_t> decodedLicense = Botan::base64_decode(licenseKey.substr(LICENSE_PREFIX.size()));
        std::string license(decodedLicense.begin(), decodedLicense.end());

        std::string line;
        std::stringstream ss(license);

        // Extract the encoded signature
        std::string encodedSignature;
        while (std::getline(ss, line)) {
            if (line.empty()) {
                break;
            }
            encodedSignature += line;
        }
        if (!line.empty()) {
            throw ParseException("Invalid license, expecting empty line");
        }

        // Decode signature. Note: it is in DER-format and needs to be converted to raw 40-byte signature
        Botan::secure_vector<std::uint8_t> decodedSignature = Botan::base64_decode(encodedSignature);
        std::vector<std::uint8_t> signature(decodedSignature.begin(), decodedSignature.end());

        // Extract license Parameters
        std::vector<std::uint8_t> message;
        std::unordered_map<std::string, std::string> parameters;
        while (std::getline(ss, line)) {
            if (line.empty()) {
                break;
            }
            std::string::size_type pos = line.find('=');
            if (pos != std::string::npos) {
                parameters[line.substr(0, pos)] = line.substr(pos + 1);
            }
            message.insert(message.end(), line.begin(), line.end());
        }

        // Decode the DSA public key
        Botan::secure_vector<std::uint8_t> decodedPublicKey(Botan::base64_decode(PUBLIC_KEY));
        Botan::AlgorithmIdentifier algId;
        std::vector<std::uint8_t> keyBits;
        Botan::BER_Decoder(decodedPublicKey).start_cons(Botan::SEQUENCE).decode(algId).decode(keyBits, Botan::BIT_STRING).end_cons();
        std::unique_ptr<Botan::Public_Key> publicKey(new Botan::DSA_PublicKey(algId, keyBits));

        // Check signature
        Botan::PK_Verifier verifier(*publicKey, "EMSA1(SHA1)", Botan::Signature_Format::DER_SEQUENCE);
        bool result = verifier.verify_message(message, signature);
        if (!result) {
            throw GenericException("Signature validation failed", ss.str());
        }

        // Success
        return parameters;
    }

    bool LicenseManager::verifyLicenseParameters(const std::unordered_map<std::string, std::string>& parameters) {
        std::string appParam, sdkProduct;
        if (!GetProductId(appParam, sdkProduct)) {
            throw GenericException("Unsupported platform");
        }

        // Get the license app identifier
        auto it = parameters.find(appParam);
        if (it != parameters.end()) {
            _appId = it->second;
        } else {
            throw GenericException("No app identifier in license");
        }

        // Check the app identifier
        std::string appId = PlatformUtils::GetAppIdentifier();
        if (!MatchAppId(it->second, appId)) {
            throw GenericException("Invalid (mismatching) app identifier: expected '" + it->second + "', actual '" + appId + "'");
        }

        // Construct the name of this product
        it = parameters.find("products");
        if (it == parameters.end()) {
            throw GenericException("No products in license");
        }

        // Check the product name
        if (!MatchProduct(it->second, sdkProduct)) {
            throw GenericException("Invalid (mismatching) product: : expected '" + it->second + "', actual '" + sdkProduct + "'");
        }

        // Check for additional required licenses
#ifdef _CARTO_GDAL_SUPPORT
        if (!MatchProduct(it->second, "sdk-gisextension")) {
            Log::Warnf("License does not support GIS extension");
        }
#endif

        // Check the watermark type, we also use this as 'license type'
        it = parameters.find("watermark");
        if (it == parameters.end()) {
            throw GenericException("Watermark not defined in license");
        }

        // Check the license valid until date, if it exists
        bool valid = true;
        it = parameters.find("validUntil");
        if (it != parameters.end()) {
            // Extract license date
            std::vector<int> expirationDate;
            for (auto it2 = boost::make_split_iterator(it->second, boost::token_finder(boost::is_any_of("-"))); it2 != boost::split_iterator<std::string::const_iterator>(); it2++) {
                try {
                    expirationDate.push_back(boost::lexical_cast<int>(*it2));
                }
                catch (const boost::bad_lexical_cast&) {
                    throw GenericException("Invalid date in license");
                }
            }

            // Get current date
            std::time_t timeT(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
            std::tm tm = { 0 };
#ifdef _WIN32
            _gmtime64_s(&tm, &timeT);
#else
            gmtime_r(&timeT, &tm);
#endif
            std::vector<int> currentDate;
            currentDate.push_back(tm.tm_year + 1900);
            currentDate.push_back(tm.tm_mon + 1);
            currentDate.push_back(tm.tm_mon);

            // Compare dates
            if (expirationDate < currentDate) {
                Log::Errorf("LicenseManager::verifyLicenseParameters: License is expired, valid until: %s", it->second.c_str());
                valid = false;
            }
        }

        // Store license type
        _parameters = parameters;
        return valid;
    }

    std::string LicenseManager::updateOnlineLicense() {
        // Request new license from server
        std::map<std::string, std::string> params;
        params["device"] = PlatformUtils::GetDeviceId();
        params["platform"] = PlatformUtils::GetPlatformId();
        std::string url = NetworkUtils::BuildURLFromParameters(LICENSE_SERVICE_URL + NetworkUtils::URLEncode(PlatformUtils::GetAppIdentifier()) + "/getLicense", params);
        std::shared_ptr<BinaryData> responseData;
        Log::Debug("LicenseManager::updateOnlineLicense: Requesting updated license");
        try {
            HTTPClient client(false);
            client.setTimeout(LICENSESERVER_TIMEOUT);
            std::map<std::string, std::string> requestHeaders;
            std::map<std::string, std::string> responseHeaders;
            if (client.get(url, requestHeaders, responseHeaders, responseData) != 0) {
                std::string result;
                if (responseData) {
                    result = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
                }
                Log::Warnf("LicenseManager::updateOnlineLicense: Failed to update license: %s", result.c_str());
                return std::string();
            }
        }
        catch (const std::exception& ex) {
            Log::Warnf("LicenseManager::updateOnlineLicense: Exception while updating license: %s", ex.what());
            return std::string();
        }

        Log::Debug("LicenseManager::updateOnlineLicense: Received updated license");
        std::string licenseKey;
        if (responseData) {
            std::string responseString = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
            picojson::value responseDoc;
            std::string err = picojson::parse(responseDoc, responseString);
            if (!err.empty()) {
                Log::Warnf("LicenseManager::updateOnlineLicense: Illegal response: %s", err.c_str());
                return std::string();
            }
            if (responseDoc.is<picojson::value::object>()) {
                const picojson::value& licenseKeyValue = responseDoc.get("license_key");
                if (licenseKeyValue.is<std::string>()) {
                    licenseKey = licenseKeyValue.get<std::string>();
                } else {
                    std::string message = "(Unknown error)";
                    const picojson::value& messageValue = responseDoc.get("license_key");
                    if (messageValue.is<std::string>()) {
                        message = messageValue.get<std::string>();
                    }
                    Log::Warnf("LicenseManager::updateOnlineLicense: Received error: %s", message.c_str());
                    return std::string();
                }
            } else {
                Log::Warnf("LicenseManager::updateOnlineLicense: Illegal response, expecting object");
                return std::string();
            }
        }

        std::lock_guard<std::mutex> lock(_mutex);

        bool result = false;
        std::unordered_map<std::string, std::string> parameters;
        try {
            parameters = DecodeLicense(licenseKey);
            result = verifyLicenseParameters(parameters);
            if (result) {
                Log::Info("LicenseManager::updateOnlineLicense: License updated successfully");
            } else {
                _parameters["watermark"] = "expired";
            }
        }
        catch (const std::exception& ex) {
            _parameters["watermark"] = "expired";
            Log::Errorf("LicenseManager::updateOnlineLicense: Invalid license: %s", ex.what());
            return std::string();
        }

        return licenseKey;
    }

    const int LicenseManager::LICENSESERVER_TIMEOUT = 5000;
    
    const std::string LicenseManager::LICENSE_PREFIX = "X";
        
    const std::string LicenseManager::LICENSE_SERVICE_URL = "https://mobile-licenseserver.carto.com/api/";

    const std::string LicenseManager::PUBLIC_KEY = "MIIBtjCCASsGByqGSM44BAEwggEeAoGBAIlFs9OqwdM4lcfhXQVCyYDzPc6rO6sbtlKuEYa/uSzeuk1tvYcGybBwB3mYLYvOh0Qd/65TUO6rYI9xTAHK0HtjGj+XLaNDiP4eEvaVfg2fhX26XDdaAGXfKWKCHKiZ0cWLnBtTap2woVvSt6TLxcxrDnwG9mrL3Lt06rflF4oJAhUAyWhMdWcGzgq37TUJcKauhX7OEHcCgYAsXe5Q9JRA3yQsAGijSifQS8Pth1FfF69dU7VUeYD55VZ5x4UVAQP+wg7K5e6RQgJpaL1R4duVkFRgr3RuTwevv1szi/3ENiIQW4vNhPxc/sN+Y2YdlNnguOhV2LEcYmneX+F5cb2UXQZBBDhVgEtU7c9bxyA6tSwKuC70EqfZSQOBhAACgYAzp7pUQ1XxR79N8NbaB1PUPE7n1bZdFLF1QsK9thjL4Q3dbg6fub3qZfSPL286nZjfD+15oya1ORFKwSplindHNx6vSL+AmNhI4GYdyIasPnLAqCV9rIMTgQ+RfmyWDvSLxSDVqWqA83M6m/FFuWMKWKqMOEueJZTwrr/HNNTk+w==";

    const std::string LicenseManager::PACKAGE_ENCRYPTION_KEY = "Hkz2jexmz9";
    
    const std::string LicenseManager::PRODUCT_VERSION = "4.0.0";

}
