#include "BitmapUtils.h"
#include "core/BinaryData.h"
#include "utils/AssetUtils.h"
#include "graphics/Bitmap.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"

#include <stdio.h>

namespace carto {

    std::shared_ptr<Bitmap> BitmapUtils::LoadBitmapFromAssets(const std::string& assetPath) {
        std::shared_ptr<BinaryData> data = AssetUtils::LoadAsset(assetPath);
        if (!data) {
            return std::shared_ptr<Bitmap>();
        }
        return Bitmap::CreateFromCompressed(data);
    }

    std::shared_ptr<Bitmap> BitmapUtils::LoadBitmapFromFile(const std::string& filePath) {
        FILE* fpRaw = fopen(filePath.c_str(), "rb");
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
        static const unsigned int ARGB_REMAP[] = { 1, 2, 3, 0 };
        static const unsigned int ABGR_REMAP[] = { 3, 2, 1, 0 };
        static const unsigned int RGBA_REMAP[] = { 0, 1, 2, 3 };
        static const unsigned int BGRA_REMAP[] = { 2, 1, 0, 3 };
        static const unsigned int RGB_INDEX_REMAP[] = { };

        if (!image) {
            return std::shared_ptr<Bitmap>();
        }
    	
        CGImageRef cgImage = image.CGImage;
        CGColorSpaceRef colorSpace = CGImageGetColorSpace(cgImage);
        CGBitmapInfo info = CGImageGetBitmapInfo(cgImage);
        unsigned int width = static_cast<unsigned int>(CGImageGetWidth(cgImage));
        unsigned int height = static_cast<unsigned int>(CGImageGetHeight(cgImage));
        unsigned int bytesPerPixel = static_cast<unsigned int>(CGImageGetBitsPerPixel(cgImage) / 8);
        unsigned int bytesPerComponent = static_cast<unsigned int>(CGImageGetBitsPerComponent(cgImage) / 8);
        unsigned int bytesPerRow = static_cast<unsigned int>(CGImageGetBytesPerRow(cgImage));

        std::vector<std::uint8_t> colorTable(CGColorSpaceGetColorTableCount(colorSpace) * 3);
        if (CGColorSpaceGetModel(colorSpace) == kCGColorSpaceModelIndexed) {
            CGColorSpaceRef baseColorSpace = CGColorSpaceGetBaseColorSpace(colorSpace);
            if (bytesPerComponent != 1 || bytesPerPixel != 1 || CGColorSpaceGetModel(baseColorSpace) != kCGColorSpaceModelRGB) {
                Log::Error("BitmapUtils::CreateBitmapFromUImage: Failed to create Bitmap, unsupported indexed format");
                return std::shared_ptr<Bitmap>();
            }
            CGColorSpaceGetColorTable(colorSpace, colorTable.data());
        }
        
        if (bytesPerComponent != 1) {
            Log::Errorf("BitmapUtils::CreateBitmapFromUImage: Failed to create Bitmap, unsupported bytes per component parameter: %u", bytesPerComponent);
            return std::shared_ptr<Bitmap>();
        }
        
        const unsigned int* rgbaRemap = RGBA_REMAP;
        ColorFormat::ColorFormat colorFormat = ColorFormat::COLOR_FORMAT_RGBA;
        bool premultiplyAlpha = false;
        switch (bytesPerPixel) {
            case 4:
                switch(info & kCGBitmapAlphaInfoMask) {
                    case kCGImageAlphaPremultipliedFirst:
                    case kCGImageAlphaFirst:
                    case kCGImageAlphaNoneSkipFirst:
                        rgbaRemap = (info & kCGBitmapByteOrderMask) == kCGBitmapByteOrder32Big ? ARGB_REMAP : BGRA_REMAP;
                        break;
                    default:
                        rgbaRemap = (info & kCGBitmapByteOrderMask) == kCGBitmapByteOrder32Big ? ABGR_REMAP : RGBA_REMAP;
                        break;
                }
                switch(info & kCGBitmapAlphaInfoMask) {
                    case kCGImageAlphaFirst:
                    case kCGImageAlphaLast:
                        premultiplyAlpha = true;
                        break;
                    default:
                        premultiplyAlpha = false;
                        break;
                }
                colorFormat = ColorFormat::COLOR_FORMAT_RGBA;
                break;
            case 3:
                colorFormat = ColorFormat::COLOR_FORMAT_RGB;
                break;
            case 2:
                colorFormat = ColorFormat::COLOR_FORMAT_GRAYSCALE_ALPHA;
                break;
            case 1:
                if (!colorTable.empty()) {
                    colorFormat = ColorFormat::COLOR_FORMAT_RGB;
                    rgbaRemap = RGB_INDEX_REMAP;
                } else {
                    colorFormat = ColorFormat::COLOR_FORMAT_GRAYSCALE;
                }
                break;
            default:
                Log::Errorf("BitmapUtils::CreateBitmapFromUImage: Failed to create Bitmap, unsupported bytes per pixel parameter: %u", bytesPerPixel);
                return std::shared_ptr<Bitmap>();
        }
    	
        CGDataProviderRef provider = CGImageGetDataProvider(cgImage);
        CFDataRef data = CGDataProviderCopyData(provider);
        const unsigned char* bytes = CFDataGetBytePtr(data);
    	
        std::shared_ptr<Bitmap> bitmap;
        if (!premultiplyAlpha && rgbaRemap == RGBA_REMAP) {
            bitmap = std::make_shared<Bitmap>(bytes, width, height, colorFormat, bytesPerRow);
        } else if (rgbaRemap == RGB_INDEX_REMAP) {
            std::vector<unsigned char> bytesUnpacked(width * height * 3);
            for (unsigned int y = 0; y < height; y++) {
                for (unsigned int x = 0; x < width; x++) {
                    unsigned int val = bytes[y * bytesPerRow + x];
                    for (unsigned int c = 0; c < 3; c++) {
                        bytesUnpacked[(y * width + x) * 3 + c] = colorTable[val * 3 + c];
                    }
                }
            }
            bitmap = std::make_shared<Bitmap>(bytesUnpacked.data(), width, height, colorFormat, width * 3);
        } else {
            std::vector<unsigned char> bytesPremultiplied(width * height * 4);
            for (unsigned int y = 0; y < height; y++) {
                for (unsigned int x = 0; x < width; x++) {
                    size_t i = y * bytesPerRow + x * 4;
                    unsigned int alpha = bytes[i + rgbaRemap[3]];
                    for (unsigned int c = 0; c < 3; c++) {
                        unsigned int val = bytes[i + rgbaRemap[c]];
                        if (premultiplyAlpha) {
                            val = (val * alpha + 255) >> 8;
                        }
                        bytesPremultiplied[(y * width + x) * 4 + c] = static_cast<unsigned char>(val);
                    }
                    bytesPremultiplied[(y * width + x) * 4 + 3] = static_cast<unsigned char>(alpha);
                }
            }
            bitmap = std::make_shared<Bitmap>(bytesPremultiplied.data(), width, height, colorFormat, width * 4);
        }
    	
        // Cleanup
        CFRelease(data);
        
        return bitmap;
    }
        
    UIImage* BitmapUtils::CreateUIImageFromBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            return nil;
        }

        CGBitmapInfo bitmapInfo;
        CGColorSpaceRef colorSpace;
        switch (bitmap->getColorFormat()) {
            case ColorFormat::COLOR_FORMAT_GRAYSCALE:
                bitmapInfo = kCGImageAlphaNone;
                colorSpace = CGColorSpaceCreateDeviceGray();
                break;
            case ColorFormat::COLOR_FORMAT_GRAYSCALE_ALPHA:
                bitmapInfo = kCGImageAlphaPremultipliedLast;
                colorSpace = CGColorSpaceCreateDeviceGray();
                break;
            case ColorFormat::COLOR_FORMAT_RGB:
                bitmapInfo = kCGImageAlphaNone;
                colorSpace = CGColorSpaceCreateDeviceRGB();
                break;
            case ColorFormat::COLOR_FORMAT_RGBA:
                bitmapInfo = kCGImageAlphaPremultipliedLast;
                colorSpace = CGColorSpaceCreateDeviceRGB();
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
        
        CFDataRef data = CFDataCreate(NULL, pixelData.data(), pixelData.size());
        CGDataProviderRef dataProvider = CGDataProviderCreateWithCFData(data);
        CGImageRef cgImage = CGImageCreate(bitmap->getWidth(), bitmap->getHeight(), 8, bitmap->getBytesPerPixel() * 8, bytesPerRow, colorSpace, bitmapInfo, dataProvider, NULL, NO, kCGRenderingIntentDefault);
        UIImage* image = [UIImage imageWithCGImage:cgImage];

        CGImageRelease(cgImage);
        CGDataProviderRelease(dataProvider);
        CFRelease(data);
        CGColorSpaceRelease(colorSpace);

        return image;
    }

    BitmapUtils::BitmapUtils() {
    }
    
}
