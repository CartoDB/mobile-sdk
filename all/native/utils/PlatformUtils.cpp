#include "PlatformUtils.h"

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

}

