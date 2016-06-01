#include "utils/PlatformUtils.h"
#include "utils/Log.h"

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <sys/utsname.h>

namespace carto {

    PlatformType::PlatformType PlatformUtils::GetPlatformType() {
#ifdef TARGET_XAMARIN
        return PlatformType::PLATFORM_TYPE_XAMARIN_IOS;
#else
        return PlatformType::PLATFORM_TYPE_IOS;
#endif
    }
    
    std::string PlatformUtils::GetDeviceId() {
#if TARGET_IPHONE_SIMULATOR
        NSString* uuid = @"FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF";
#else
        NSString* uuid = [[UIDevice currentDevice] identifierForVendor].UUIDString;
#endif
        return std::string([uuid UTF8String]);
    }

    std::string PlatformUtils::GetDeviceType() {
        struct utsname systemInfo;
        memset(&systemInfo, 0, sizeof(systemInfo));
        uname(&systemInfo);
        return systemInfo.machine;
    }

    std::string PlatformUtils::GetDeviceOS() {
        NSString* version = [[UIDevice currentDevice] systemVersion];
        if (version) {
            return std::string([version UTF8String]);
        }
        return "iOS Unknown";
    }
    
    std::string PlatformUtils::GetAppIdentifier() {
        NSString* bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];
        return std::string([bundleIdentifier UTF8String]);
    }
    
    std::string PlatformUtils::GetAppDeviceId() {
        return GetDeviceId() + GetAppIdentifier();
    }
    
    bool PlatformUtils::ExcludeFolderFromBackup(const std::string &folder) {
        // Exclude map packages from iCloud backup, important as device id may change after restore
        NSString* directory = [[NSString alloc] initWithUTF8String:folder.c_str()];
        NSURL* url = [NSURL fileURLWithPath:directory];
        NSError* error;
        return [url setResourceValue:[NSNumber numberWithBool:YES] forKey:NSURLIsExcludedFromBackupKey error:&error] == YES;
    }

    PlatformUtils::PlatformUtils() {
    }

}
