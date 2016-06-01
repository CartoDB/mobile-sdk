#include "utils/PlatformUtils.h"
#include "utils/Log.h"

#include <algorithm>

#include <utf8.h>

namespace carto {

    PlatformType::PlatformType PlatformUtils::GetPlatformType() {
        return PlatformType::PLATFORM_TYPE_WINDOWS_PHONE;
    }
    
    std::string PlatformUtils::GetDeviceId() {
        std::wstring wid;
        try {
            Windows::System::Profile::HardwareToken^ token = Windows::System::Profile::HardwareIdentification::GetPackageSpecificToken(nullptr);
            Windows::Storage::Streams::IBuffer^ hardwareId = token->Id;

            Windows::Security::Cryptography::Core::HashAlgorithmProvider^ hasher = Windows::Security::Cryptography::Core::HashAlgorithmProvider::OpenAlgorithm(Windows::Security::Cryptography::Core::HashAlgorithmNames::Md5);
            Windows::Storage::Streams::IBuffer^ hashed = hasher->HashData(hardwareId);

            wid = Windows::Security::Cryptography::CryptographicBuffer::EncodeToHexString(hashed)->Data();
        }
        catch (Platform::Exception^ e) {
            Log::Error("PlatformUtils::GetDeviceId: Exception while reading DeviceId");
        }
        std::string id;
        utf8::utf16to8(wid.begin(), wid.end(), std::back_inserter(id));
        return id;
    }

    std::string PlatformUtils::GetDeviceType() {
        std::wstring wdeviceType;
        try {
            Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation^ deviceInfo = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
            wdeviceType = deviceInfo->SystemSku->Data();
        }
        catch (Platform::Exception^ e) {
           Log::Error("PlatformUtils::GetDeviceType: Exception while reading device info");
        }
        std::string deviceType;
        utf8::utf16to8(wdeviceType.begin(), wdeviceType.end(), std::back_inserter(deviceType));
        return deviceType;
    }
    
    std::string PlatformUtils::GetDeviceOS() {
        std::wstring wdeviceOS;
        try {
            Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation^ deviceInfo = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
            wdeviceOS = deviceInfo->OperatingSystem->Data();
        }
        catch (Platform::Exception^ e) {
           Log::Error("PlatformUtils::GetDeviceOS: Exception while reading device info");
        }
        std::string deviceOS;
        utf8::utf16to8(wdeviceOS.begin(), wdeviceOS.end(), std::back_inserter(deviceOS));
        return deviceOS;
    }
    
    std::string PlatformUtils::GetAppIdentifier() {
        std::wstring wid;
        try {
            wid = Windows::ApplicationModel::Store::CurrentApp::AppId.ToString()->Data();
        }
        catch (Platform::Exception^ e) {
           Log::Error("PlatformUtils::GetAppIdentifier: Exception while reading AppId");
        }
        std::string id;
        utf8::utf16to8(wid.begin(), wid.end(), std::back_inserter(id));
        id.erase(std::remove(id.begin(), id.end(), '{'), id.end());
        id.erase(std::remove(id.begin(), id.end(), '}'), id.end());
        return id;
    }
    
    std::string PlatformUtils::GetAppDeviceId() {
        return GetDeviceId() + GetAppIdentifier();
    }
    
    bool PlatformUtils::ExcludeFolderFromBackup(const std::string &folder) {
        // This is iOS-specific method, simply ignore it on Windows Phone
        return true;
    }

    PlatformUtils::PlatformUtils() {
    }

}
