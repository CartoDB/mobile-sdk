#include "utils/PlatformUtils.h"
#include "utils/AndroidUtils.h"
#include "utils/Log.h"

namespace carto {

    PlatformType::PlatformType PlatformUtils::GetPlatformType() {
#ifdef TARGET_XAMARIN
        return PlatformType::PLATFORM_TYPE_XAMARIN_ANDROID;
#else
        return PlatformType::PLATFORM_TYPE_ANDROID;
#endif
    }
    
    std::string PlatformUtils::GetDeviceId() {
        return AndroidUtils::GetDeviceId();
    }

    std::string PlatformUtils::GetDeviceType() {
        return AndroidUtils::GetDeviceType();
    }
    
    std::string PlatformUtils::GetDeviceOS() {
        return AndroidUtils::GetDeviceOS();
    }
    
    std::string PlatformUtils::GetAppIdentifier() {
        return AndroidUtils::GetPackageName();
    }
    
    std::string PlatformUtils::GetAppDeviceId() {
        std::string appId = AndroidUtils::GetPackageName();
        std::string deviceId = AndroidUtils::GetDeviceId();
        if (deviceId.empty()) {
            return appId;
        }
        return appId + ":" + deviceId;
    }
    
    bool PlatformUtils::ExcludeFolderFromBackup(const std::string &folder) {
        // This is iOS-specific method, simply ignore it on Android
        return true;
    }

    PlatformUtils::PlatformUtils() {
    }

}
