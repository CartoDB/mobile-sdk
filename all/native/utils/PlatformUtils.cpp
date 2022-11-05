#include "PlatformUtils.h"

#define EXPAND(str) #str
#define STRINGIFY(str) EXPAND(str)

namespace carto {

    std::string PlatformUtils::GetPlatformId() {
        switch (GetPlatformType()) {
        case PlatformType::PLATFORM_TYPE_ANDROID:
            return "android";
        case PlatformType::PLATFORM_TYPE_IOS:
            return "ios";
        case PlatformType::PLATFORM_TYPE_XAMARIN_ANDROID:
            return "xamarin-android";
        case PlatformType::PLATFORM_TYPE_XAMARIN_IOS:
            return "xamarin-ios";
        case PlatformType::PLATFORM_TYPE_WINDOWS_PHONE:
            return "windows-phone";
        default:
            return "";
        }
    }

    std::string PlatformUtils::GetSDKVersion() {
        return STRINGIFY(_CARTO_MOBILE_SDK_VERSION);
    }

}

