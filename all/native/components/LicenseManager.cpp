#include "LicenseManager.h"
#include "core/BinaryData.h"
#include "components/LicenseManagerListener.h"
#include "utils/PlatformUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <base64.h>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <dsa.h>
#include <filters.h>
#include <iomanip>
#include <randpool.h>
#include <regex>
#include <sstream>
#include <tuple>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <picojson/picojson.h>

namespace carto {

    bool LicenseManager::registerLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener) {
        std::unordered_map<std::string, std::string> parameters;
        try {
            parameters = DecodeLicense(licenseKey);
        }
        catch (const std::exception& ex) {
            Log::Errorf("LicenseManager::registerLicense: Invalid license: %s", ex.what());
            return false;
        }

        std::lock_guard<std::mutex> lock(_mutex);
        bool result = verifyLicenseParameters(parameters);
        if (!result) {
            _watermarkType = EXPIRED_WATERMARK;
        }
        
        auto it = parameters.find("onlineLicense");
        if (it != parameters.end()) {
            if (!it->second.empty() && it->second != "0") {
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

    std::string LicenseManager::getUserKey() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _userKey;
    }

    LicenseManager::WatermarkType LicenseManager::getWatermarkType() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _watermarkType;
    }
    
    LicenseManager& LicenseManager::GetInstance() {
        static LicenseManager instance;
        return instance;
    }

    LicenseManager::LicenseManager() :
        _appId(),
        _userKey(),
        _watermarkType(EVALUATION_WATERMARK),
        _updateThreads(),
        _mutex()
    {
    }

    LicenseManager::~LicenseManager() {
        for (std::thread& updateThread : _updateThreads) {
            updateThread.join();
        }
    }

    bool LicenseManager::GetProductPlatformId(std::string& appParam, std::string& sdkProduct, std::string& platformId) {
        switch (PlatformUtils::GetPlatformType()) {
        case PlatformType::PLATFORM_TYPE_ANDROID:
            appParam = "packageName";
            sdkProduct = "sdk-android-";
            platformId = "Android";
            break;
        case PlatformType::PLATFORM_TYPE_IOS:
            appParam = "bundleIdentifier";
            sdkProduct = "sdk-ios-";
            platformId = "iOS";
            break;
        case PlatformType::PLATFORM_TYPE_XAMARIN_ANDROID:
            appParam = "packageName";
            sdkProduct = "sdk-xamarin-android-";
            platformId = "Xamarin Android";
            break;
        case PlatformType::PLATFORM_TYPE_XAMARIN_IOS:
            appParam = "bundleIdentifier";
            sdkProduct = "sdk-xamarin-ios-";
            platformId = "Xamarin iOS";
            break;
        case PlatformType::PLATFORM_TYPE_WINDOWS_PHONE:
            appParam = "productId";
            sdkProduct = "sdk-winphone-";
            platformId = "Windows Phone";
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
            throw std::runtime_error("Invalid license prefix");
        }
        std::string decodedLicense;
        CryptoPP::Base64Decoder* decoder = new CryptoPP::Base64Decoder(new CryptoPP::StringSink(decodedLicense));
        CryptoPP::StringSource(licenseKey.substr(LICENSE_PREFIX.size()), true, decoder);

        std::string line;
        std::stringstream ss(decodedLicense);

        // Extract the encoded signature
        std::string encodedSignature;
        while (std::getline(ss, line)) {
            if (line.empty()) {
                break;
            }
            encodedSignature += line;
        }
        if (!line.empty()) {
            throw std::runtime_error("Invalid license, expecting empty line");
        }

        // Decode signature. Note: it is in DER-format and needs to be converted to raw 40-byte signature
        std::string decodedSignature;
        decoder = new CryptoPP::Base64Decoder(new CryptoPP::StringSink(decodedSignature));
        CryptoPP::StringSource(encodedSignature, true, decoder);

        // Extract license Parameters
        std::string message;
        std::unordered_map<std::string, std::string> parameters;
        while (std::getline(ss, line)) {
            if (line.empty()) {
                break;
            }
            std::string::size_type pos = line.find('=');
            if (pos != std::string::npos) {
                parameters[line.substr(0, pos)] = line.substr(pos + 1);
            }
            message += line;
        }

        // Decode the public key
        std::string decodedPublicKey;
        decoder = new CryptoPP::Base64Decoder(new CryptoPP::StringSink(decodedPublicKey));
        CryptoPP::StringSource ww(PUBLIC_KEY, true, decoder);

        // Load and validate DSA public key
        CryptoPP::DSA::PublicKey publicKey;
        publicKey.Load(CryptoPP::StringSource(decodedPublicKey, true).Ref());
        CryptoPP::RandomPool rnd;
        if (!publicKey.Validate(rnd, 2)) {
            throw std::runtime_error("Public key validation failed");
        }

        // Check signature, first convert signature from DER to P1364 format
        CryptoPP::DSA::Verifier verifier(publicKey);
        char signature[1024]; // DSA_P1364 signatures are actually 40 bytes
        CryptoPP::DSAConvertSignatureFormat((byte*)signature, verifier.SignatureLength(), CryptoPP::DSA_P1363, (const byte*)decodedSignature.c_str(), decodedSignature.size(), CryptoPP::DSA_DER);

        bool result = false;
        CryptoPP::SignatureVerificationFilter* verificationFilter = new CryptoPP::SignatureVerificationFilter(
            verifier,
            new CryptoPP::ArraySink((byte*)&result, sizeof(result)),
            CryptoPP::SignatureVerificationFilter::SIGNATURE_AT_END | CryptoPP::SignatureVerificationFilter::PUT_RESULT);
        CryptoPP::StringSource(message + std::string(signature, signature + verifier.SignatureLength()), true, verificationFilter);
        if (!result) {
            throw std::runtime_error("Signature validation failed: " + ss.str());
        }

        // Success
        return parameters;
    }

    bool LicenseManager::verifyLicenseParameters(const std::unordered_map<std::string, std::string>& parameters) {
        std::string appParam, sdkProduct, platformId;
        if (!GetProductPlatformId(appParam, sdkProduct, platformId)) {
            Log::Error("LicenseManager::verifyLicenseParameters: Unsupported platform");
            return false;
        }

        // Get the license app identifier
        auto it = parameters.find(appParam);
        if (it != parameters.end()) {
            _appId = it->second;
        } else {
            Log::Error("LicenseManager::verifyLicenseParameters: No app identifier");
            return false;
        }

        // Check the app identifier
        if (!MatchAppId(it->second, PlatformUtils::GetAppIdentifier())) {
            Log::Error("LicenseManager::verifyLicenseParameters: Invalid app identifier");
            return false;
        }

        // Construct the name of this product (SDK 3)
        it = parameters.find("products");
        if (it == parameters.end()) {
            Log::Error("LicenseManager::verifyLicenseParameters: No products");
            return false;
        }

        // Check the product name
        if (!MatchProduct(it->second, sdkProduct)) {
            Log::Error("LicenseManager::verifyLicenseParameters: Invalid product");
            return false;
        }

        // Check for additional required licenses
#ifdef _CARTO_GDAL_SUPPORT
        if (!MatchProduct(it->second, "sdk-gisextension")) {
            Log::Error("LicenseManager::verifyLicenseParameters: License does not support GIS extension");
            return false;
        }
#endif

        // Check the watermark type, we also use this as 'license type'
        it = parameters.find("watermark");
        if (it == parameters.end()) {
            Log::Error("LicenseManager::verifyLicenseParameters: No watermark");
            return false;
        }
        WatermarkType watermarkType = EVALUATION_WATERMARK;
        if (it->second == "nutiteq") {
            watermarkType = CARTO_WATERMARK;
        } else if (it->second == "evaluation") {
            watermarkType = EVALUATION_WATERMARK;
        } else if (it->second == "custom") {
            watermarkType = CUSTOM_WATERMARK;
        } else {
            Log::Error("LicenseManager::verifyLicenseParameters: Invalid watermark");
            return false;
        }

        // Store user key. Even if license is expired, we will store this.
        _userKey.clear();
        it = parameters.find("userKey");
        if (it != parameters.end()) {
            _userKey = it->second;
        }

        // Check the license valid until date, if it exists
        it = parameters.find("validUntil");
        if (it != parameters.end()) {
            // Extract license date
            std::vector<int> expirationDate;
            for (auto it2 = boost::make_split_iterator(it->second, boost::token_finder(boost::is_any_of("-"))); it2 != boost::split_iterator<std::string::const_iterator>(); it2++) {
                try {
                    expirationDate.push_back(boost::lexical_cast<int>(*it2));
                } catch (const boost::bad_lexical_cast&) {
                    Log::Error("LicenseManager::verifyLicenseParameters: Invalid date");
                    return false;
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
                Log::Error("LicenseManager::verifyLicenseParameters: License is expired");
                return false;
            }
        }

        // Store license type
        _watermarkType = watermarkType;
        return true;
    }

    std::string LicenseManager::updateOnlineLicense() {
        std::string appParam, sdkProduct, platformId;
        GetProductPlatformId(appParam, sdkProduct, platformId);

        // Request new license from server
        std::map<std::string, std::string> params;
        params["device"] = PlatformUtils::GetDeviceId();
        params["user_key"] = getUserKey();
        params["platform"] = platformId;
        std::string url = NetworkUtils::BuildURLFromParameters(LICENSESERVER_URL + NetworkUtils::URLEncode(PlatformUtils::GetAppIdentifier()) + "/getUserLicense", params);
        std::shared_ptr<BinaryData> responseData;
        if (!NetworkUtils::GetHTTP(url, responseData, false)) {
            Log::Warnf("LicenseManager::updateOnlineLicense: Failed to update license");
            return std::string();
        }
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
                const picojson::value& licenseCode = responseDoc.get("license_code");
                if (licenseCode.is<std::string>()) {
                    licenseKey = licenseCode.get<std::string>();
                }
            }
        }

        std::unordered_map<std::string, std::string> parameters;
        try {
            parameters = DecodeLicense(licenseKey);
        }
        catch (const std::exception& ex) {
            Log::Errorf("LicenseManager::updateOnlineLicense: Invalid license: %s", ex.what());
            return std::string();
        }

        std::lock_guard<std::mutex> lock(_mutex);
        bool result = verifyLicenseParameters(parameters);
        if (!result) {
            _watermarkType = EXPIRED_WATERMARK;
        }

        Log::Info("LicenseManager::updateOnlineLicense: License updated");
        return licenseKey;
    }
    
    const std::string LicenseManager::LICENSE_PREFIX = "X";
        
    const std::string LicenseManager::LICENSESERVER_URL = "https://service.nutiteq.com/nutiteqsdk/";

    const std::string LicenseManager::PUBLIC_KEY = "MIIBtjCCASsGByqGSM44BAEwggEeAoGBAIlFs9OqwdM4lcfhXQVCyYDzPc6rO6sbtlKuEYa/uSzeuk1tvYcGybBwB3mYLYvOh0Qd/65TUO6rYI9xTAHK0HtjGj+XLaNDiP4eEvaVfg2fhX26XDdaAGXfKWKCHKiZ0cWLnBtTap2woVvSt6TLxcxrDnwG9mrL3Lt06rflF4oJAhUAyWhMdWcGzgq37TUJcKauhX7OEHcCgYAsXe5Q9JRA3yQsAGijSifQS8Pth1FfF69dU7VUeYD55VZ5x4UVAQP+wg7K5e6RQgJpaL1R4duVkFRgr3RuTwevv1szi/3ENiIQW4vNhPxc/sN+Y2YdlNnguOhV2LEcYmneX+F5cb2UXQZBBDhVgEtU7c9bxyA6tSwKuC70EqfZSQOBhAACgYAzp7pUQ1XxR79N8NbaB1PUPE7n1bZdFLF1QsK9thjL4Q3dbg6fub3qZfSPL286nZjfD+15oya1ORFKwSplindHNx6vSL+AmNhI4GYdyIasPnLAqCV9rIMTgQ+RfmyWDvSLxSDVqWqA83M6m/FFuWMKWKqMOEueJZTwrr/HNNTk+w==";
    
    const std::string LicenseManager::PRODUCT_VERSION = "3.x.y";

}
