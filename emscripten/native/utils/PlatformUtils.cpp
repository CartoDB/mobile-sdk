#include "utils/PlatformUtils.h"

namespace carto {

    PlatformType::PlatformType PlatformUtils::GetPlatformType() {
        return PlatformType::PLATFORM_TYPE_WEB;
    }
    
    std::string PlatformUtils::GetDeviceId() {
        return "deviceId";
    }

    std::string PlatformUtils::GetDeviceType() {
        return "web";
    }
    
    std::string PlatformUtils::GetDeviceOS() {
        return "web";
    }
    
    std::string PlatformUtils::GetAppIdentifier() {
        return "web";
    }
    
    std::string PlatformUtils::GetAppDeviceId() {
        return "web:web";
    }
    
    bool PlatformUtils::ExcludeFolderFromBackup(const std::string &folder) {
        return true;
    }

    PlatformUtils::PlatformUtils() {
    }

}
