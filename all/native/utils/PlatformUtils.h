/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PLATFORMUTILS_H_
#define _CARTO_PLATFORMUTILS_H_

#include <string>
#include <mutex>

namespace carto {
    
    namespace PlatformType {
        enum PlatformType {
            PLATFORM_TYPE_ANDROID,
            PLATFORM_TYPE_IOS,
            PLATFORM_TYPE_MAC_OS,
            PLATFORM_TYPE_WINDOWS,
            PLATFORM_TYPE_WINDOWS_PHONE,
            PLATFORM_TYPE_XAMARIN_IOS,
            PLATFORM_TYPE_XAMARIN_ANDROID
        };
    }
    
    class PlatformUtils {
    public:
        static PlatformType::PlatformType GetPlatformType();

        static std::string GetPlatformId();
        
        static std::string GetDeviceId();

        static std::string GetDeviceType();

        static std::string GetDeviceOS();

        static std::string GetAppIdentifier();

        static std::string GetAppDeviceId();

        static std::string GetSDKVersion();

        static bool ExcludeFolderFromBackup(const std::string& folder);
        
    private:
        PlatformUtils();
    };
    
}

#endif
