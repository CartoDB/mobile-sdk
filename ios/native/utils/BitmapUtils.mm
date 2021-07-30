#include "BitmapUtils.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "utils/AssetUtils.h"
#include "graphics/Bitmap.h"
#include "utils/CFUniquePtr.h"
#include "utils/Log.h"

#include <stdext/utf8_filesystem.h>

namespace carto {

    std::shared_ptr<Bitmap> BitmapUtils::LoadBitmapFromAssets(const std::string& assetPath) {
        std::shared_ptr<BinaryData> data = AssetUtils::LoadAsset(assetPath);
        if (!data) {
            return std::shared_ptr<Bitmap>();
        }
        return Bitmap::CreateFromCompressed(data);
    }

    std::shared_ptr<Bitmap> BitmapUtils::LoadBitmapFromFile(const std::string& filePath) {
        FILE* fpRaw = utf8_filesystem::fopen(filePath.c_str(), "rb");
        if (!fpRaw) {
            Log::Errorf("BitmapUtils::LoadBitmapFromFile: Failed to load: %s", filePath.c_str());
            return std::shared_ptr<Bitmap>();
        }
        std::shared_ptr<FILE> fp(fpRaw, fclose);
        fseek(fp.get(), 0, SEEK_END);
        long size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);
        std::vector<unsigned char> data(static_cast<size_t>(size));
        fread(data.data(), 1, size, fp.get());
        return Bitmap::CreateFromCompressed(data.data(), data.size());
    }
    
    std::shared_ptr<Bitmap> BitmapUtils::CreateBitmapFromUIImage(const UIImage* image) {
        if (!image) {
            throw NullArgumentException("Null image");
        }
    
        CGImageRef cgImage = image.CGImage;
        CGBitmapInfo info = CGImageGetBitmapInfo(cgImage);
        unsigned int width = static_cast<unsigned int>(CGImageGetWidth(cgImage));
        unsigned int height = static_cast<unsigned int>(CGImageGetHeight(cgImage));

        std::vector<std::uint8_t> bitmapData(width * height * 4);
        CFUniquePtr<CGColorSpaceRef> colorSpace(CGColorSpaceCreateDeviceRGB(), CGColorSpaceRelease);
        CFUniquePtr<CGContextRef> context(CGBitmapContextCreate(bitmapData.data(), width, height, 8, width * 4, colorSpace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast), CGContextRelease);
        if (!context) {
            Log::Errorf("BitmapUtils::CreateBitmapFromUIImage: Failed to create bitmap context");
            return std::shared_ptr<Bitmap>();
        }

        CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);

        return std::make_shared<Bitmap>(bitmapData.data(), width, height, ColorFormat::COLOR_FORMAT_RGBA, width * 4);
    }
        
    UIImage* BitmapUtils::CreateUIImageFromBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            throw NullArgumentException("Null bitmap");
        }

        CGBitmapInfo bitmapInfo;
        CFUniquePtr<CGColorSpaceRef> colorSpace;
        switch (bitmap->getColorFormat()) {
            case ColorFormat::COLOR_FORMAT_GRAYSCALE:
                bitmapInfo = kCGImageAlphaNone;
                colorSpace = CFUniquePtr<CGColorSpaceRef>(CGColorSpaceCreateDeviceGray(), CGColorSpaceRelease);
                break;
            case ColorFormat::COLOR_FORMAT_GRAYSCALE_ALPHA:
                bitmapInfo = kCGImageAlphaPremultipliedLast;
                colorSpace = CFUniquePtr<CGColorSpaceRef>(CGColorSpaceCreateDeviceGray(), CGColorSpaceRelease);
                break;
            case ColorFormat::COLOR_FORMAT_RGB:
                bitmapInfo = kCGImageAlphaNone;
                colorSpace = CFUniquePtr<CGColorSpaceRef>(CGColorSpaceCreateDeviceRGB(), CGColorSpaceRelease);
                break;
            case ColorFormat::COLOR_FORMAT_RGBA:
                bitmapInfo = kCGImageAlphaPremultipliedLast;
                colorSpace = CFUniquePtr<CGColorSpaceRef>(CGColorSpaceCreateDeviceRGB(), CGColorSpaceRelease);
                break;
            default:
                Log::Errorf("BitmapUtils::CreateUIImageFromBitmap: Failed to create UIImage, unsupported color format: %d", bitmap->getColorFormat());
                return nil;
        }

        std::vector<unsigned char> pixelData(bitmap->getWidth() * bitmap->getHeight() * bitmap->getBytesPerPixel());
        unsigned int bytesPerRow = bitmap->getWidth() * bitmap->getBytesPerPixel();
        for (unsigned int y = 0; y < bitmap->getHeight(); y++) {
            const unsigned char* row = bitmap->getPixelData().data() + (bitmap->getHeight() - 1 - y) * bytesPerRow;
            std::copy(row, row + bytesPerRow, &pixelData[y * bytesPerRow]);
        }
        
        CFUniquePtr<CFDataRef> data(CFDataCreate(NULL, pixelData.data(), pixelData.size()), CFRelease);
        CFUniquePtr<CGDataProviderRef> dataProvider(CGDataProviderCreateWithCFData(data), CGDataProviderRelease);
        CFUniquePtr<CGImageRef> cgImage(CGImageCreate(bitmap->getWidth(), bitmap->getHeight(), 8, bitmap->getBytesPerPixel() * 8, bytesPerRow, colorSpace, bitmapInfo, dataProvider, NULL, NO, kCGRenderingIntentDefault), CGImageRelease);
        UIImage* image = [UIImage imageWithCGImage:cgImage];

        return image;
    }

    BitmapUtils::BitmapUtils() {
    }
    
}
