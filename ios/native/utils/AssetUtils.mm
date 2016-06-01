#include "AssetUtils.h"
#include "core/BinaryData.h"
#include "utils/Log.h"

#import <Foundation/Foundation.h>

namespace carto {

    std::shared_ptr<BinaryData> AssetUtils::LoadAsset(const std::string& path) {
        // Convert std::string to NSString
        NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
        NSString* fileName = [nsPath stringByDeletingPathExtension];
        NSString* extension = [nsPath pathExtension];
        NSString* fullPath = [[NSBundle mainBundle] pathForResource:fileName ofType:extension];
        
        // Try to open file for reading
        NSFileHandle* fileHandle = [NSFileHandle fileHandleForReadingAtPath:fullPath];
        if (!fileHandle) {
            Log::Errorf("AssetUtils::LoadAsset: Asset not found: %s", path.c_str());
            return std::shared_ptr<BinaryData>();
        }
        
        // Read the file
        NSData* fileData = [fileHandle readDataToEndOfFile];
        [fileHandle closeFile];
        
        if ([fileData length] < 0) {
            Log::Errorf("AssetUtils::LoadAsset: Data length <0: %s", path.c_str());
            return std::shared_ptr<BinaryData>();
        }
        
        // Copy the data to vector
        std::vector<unsigned char> data;
        const unsigned char* bytes = static_cast<const unsigned char*>([fileData bytes]);
        data.assign(bytes, bytes + [fileData length]);
        return std::make_shared<BinaryData>(std::move(data));
    }

    std::string AssetUtils::CalculateResourcePath(const std::string& resourceName) {
        NSString* nsResourceName = [NSString stringWithUTF8String:resourceName.c_str()];
        NSString* fileName = [nsResourceName stringByDeletingPathExtension];
        NSString* extension = [nsResourceName pathExtension];
        NSString* fullPath = [[NSBundle mainBundle] pathForResource:fileName ofType:extension];
        
        if (!fullPath) {
            Log::Errorf("AssetUtils::CalculateResourcePath: Asset not found: %s", resourceName.c_str());
            return std::string();
        }
        
        return std::string([fullPath UTF8String]);
    }
        
    std::string AssetUtils::CalculateWritablePath(const std::string& fileName) {
        NSArray* dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* docsDir = [dirPaths objectAtIndex:0];
        NSString* nsFileName = [NSString stringWithUTF8String:fileName.c_str()];
        NSString* writablePath = [[NSString alloc] initWithString: [docsDir stringByAppendingPathComponent: nsFileName]];
        return std::string([writablePath UTF8String]);
    }

    AssetUtils::AssetUtils() {
    }
    
}
