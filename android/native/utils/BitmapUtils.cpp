#include "BitmapUtils.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "graphics/Bitmap.h"
#include "utils/AssetUtils.h"
#include "utils/AndroidUtils.h"
#include "utils/JNILocalFrame.h"
#include "utils/Log.h"

#include <stdext/utf8_filesystem.h>

#include <android/bitmap.h>

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
        if (size <= 0) {
            Log::Errorf("BitmapUtils::LoadBitmapFromFile: Ignore load due to size %d: %s", size, filePath.c_str());
            return std::shared_ptr<Bitmap>();
        }
        std::vector<unsigned char> data(static_cast<std::size_t>(size));
        fread(data.data(), 1, size, fp.get());
        return Bitmap::CreateFromCompressed(data.data(), data.size());
    }

    std::shared_ptr<Bitmap> BitmapUtils::CreateBitmapFromAndroidBitmap(jobject androidBitmap) {
        if (!androidBitmap) {
            throw NullArgumentException("Null androidBitmap");
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();

        AndroidBitmapInfo bitmapInfo;
        if (AndroidBitmap_getInfo(jenv, androidBitmap, &bitmapInfo) != ANDROID_BITMAP_RESULT_SUCCESS) {
            Log::Error("BitmapUtils::CreateBitmapFromAndroidBitmap: Failed to read Android bitmap info");
            return std::shared_ptr<Bitmap>();
        }

        unsigned int bytesPerPixel = 0;
        ColorFormat::ColorFormat colorFormat = ColorFormat::COLOR_FORMAT_RGBA;
        switch (bitmapInfo.format) {
        case ANDROID_BITMAP_FORMAT_A_8:
            bytesPerPixel = 1;
            colorFormat = ColorFormat::COLOR_FORMAT_GRAYSCALE;
            break;
        case ANDROID_BITMAP_FORMAT_RGB_565:
            bytesPerPixel = 2;
            colorFormat = ColorFormat::COLOR_FORMAT_RGB_565;
            break;
        case ANDROID_BITMAP_FORMAT_RGBA_4444:
            bytesPerPixel = 2;
            colorFormat = ColorFormat::COLOR_FORMAT_RGBA_4444;
            break;
        case ANDROID_BITMAP_FORMAT_RGBA_8888:
            bytesPerPixel = 4;
            colorFormat = ColorFormat::COLOR_FORMAT_RGBA;
            break;
        case ANDROID_BITMAP_FORMAT_NONE:
        default:
            Log::Errorf("BitmapUtils::CreateBitmapFromAndroidBitmap: Unsupported color format: %d", bitmapInfo.format);
            return std::shared_ptr<Bitmap>();
        }

        unsigned char* uncompressedData = nullptr;
        if (AndroidBitmap_lockPixels(jenv, androidBitmap, reinterpret_cast<void **>(&uncompressedData)) != ANDROID_BITMAP_RESULT_SUCCESS) {
            Log::Error("BitmapUtils::CreateBitmapFromAndroidBitmap: Failed to lock bitmap pixels");
            return std::shared_ptr<Bitmap>();
        }
        std::shared_ptr<Bitmap> bitmap;
        try {
            bitmap = std::make_shared<Bitmap>(uncompressedData, bitmapInfo.width, bitmapInfo.height, colorFormat, bitmapInfo.width * bytesPerPixel);
        }
        catch (const std::exception& ex) {
            Log::Errorf("BitmapUtils::CreateBitmapFromAndroidBitmap: Failed to create bitmap: %s", ex.what());
            AndroidBitmap_unlockPixels(jenv, androidBitmap);
            throw;
        }
        AndroidBitmap_unlockPixels(jenv, androidBitmap);
        return bitmap;
    }

    jobject BitmapUtils::CreateAndroidBitmapFromBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            throw NullArgumentException("Null bitmap");
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();

        jobject javaBitmapGlobal = NULL;
        {
            JNILocalFrame jframe(jenv, 32, "BitmapUtils::CreateAndroidBitmapFromBitmap");
            if (!jframe.isValid()) {
                return NULL;
            }

            jclass bitmapCls = jenv->FindClass("android/graphics/Bitmap");
            jmethodID createBitmapFunction = jenv->GetStaticMethodID(bitmapCls, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
            jstring configName = jenv->NewStringUTF("ARGB_8888");
            jclass bitmapConfigClass = jenv->FindClass("android/graphics/Bitmap$Config");
            jmethodID valueOfBitmapConfigFunction = jenv->GetStaticMethodID(bitmapConfigClass, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
            jobject bitmapConfig = jenv->CallStaticObjectMethod(bitmapConfigClass, valueOfBitmapConfigFunction, configName);
            jobject javaBitmap = jenv->CallStaticObjectMethod(bitmapCls, createBitmapFunction, bitmap->getWidth(), bitmap->getHeight(), bitmapConfig);
            if (jenv->ExceptionCheck()) {
                Log::Error("BitmapUtils::CreateBitmapFromAndroidBitmap: Failed to create Android bitmap");
                return NULL;
            }

            AndroidBitmapInfo bitmapInfo;
            AndroidBitmap_getInfo(jenv, javaBitmap, &bitmapInfo);
            unsigned char* destData = nullptr;
            if (AndroidBitmap_lockPixels(jenv, javaBitmap, reinterpret_cast<void **>(&destData)) != ANDROID_BITMAP_RESULT_SUCCESS) {
                Log::Error("BitmapUtils::CreateBitmapFromAndroidBitmap: Failed to lock bitmap pixels");
                return NULL;
            }

            const unsigned char* srcData = bitmap->getPixelData().data();
            unsigned int destBytesPerPixel = 4;
            unsigned int destBytesPerRow = bitmap->getWidth() * destBytesPerPixel;
            unsigned int srcBytesPerPixel = bitmap->getBytesPerPixel();
            unsigned int srcBytesPerRow = bitmap->getWidth() * srcBytesPerPixel;
            for (unsigned int i = 0; i < bitmap->getHeight(); i++) {
                unsigned int flippedI = (bitmap->getHeight() - 1 - i);
                for (unsigned int j = 0; j < bitmap->getWidth(); j++) {
                    unsigned int destIndex = flippedI * destBytesPerRow + j * destBytesPerPixel;
                    unsigned int srcIndex = i * srcBytesPerRow + j * srcBytesPerPixel;
                    switch (bitmap->getColorFormat()) {
                    case ColorFormat::COLOR_FORMAT_GRAYSCALE:
                        destData[destIndex + 0] = srcData[srcIndex];
                        destData[destIndex + 1] = srcData[srcIndex];
                        destData[destIndex + 2] = srcData[srcIndex];
                        destData[destIndex + 3] = srcData[srcIndex];
                        break;
                    case ColorFormat::COLOR_FORMAT_GRAYSCALE_ALPHA:
                        destData[destIndex + 0] = srcData[srcIndex];
                        destData[destIndex + 1] = srcData[srcIndex];
                        destData[destIndex + 2] = srcData[srcIndex];
                        destData[destIndex + 3] = srcData[srcIndex + 1];
                        break;
                    case ColorFormat::COLOR_FORMAT_RGB:
                        destData[destIndex + 0] = srcData[srcIndex + 0];
                        destData[destIndex + 1] = srcData[srcIndex + 1];
                        destData[destIndex + 2] = srcData[srcIndex + 2];
                        destData[destIndex + 3] = 255;
                        break;
                    case ColorFormat::COLOR_FORMAT_RGBA:
                        destData[destIndex + 0] = srcData[srcIndex + 0];
                        destData[destIndex + 1] = srcData[srcIndex + 1];
                        destData[destIndex + 2] = srcData[srcIndex + 2];
                        destData[destIndex + 3] = srcData[srcIndex + 3];
                        break;
                    default:
                        Log::Error("BitmapUtils::CreateAndroidBitmapFromBitmap: Failed to convert bitmap");
                        AndroidBitmap_unlockPixels(jenv, javaBitmap);
                        return NULL;
                    }
                }
            }

            AndroidBitmap_unlockPixels(jenv, javaBitmap);

            javaBitmapGlobal = jenv->NewGlobalRef(javaBitmap);
        }

        jobject javaBitmap = jenv->NewLocalRef(javaBitmapGlobal);
        jenv->DeleteGlobalRef(javaBitmapGlobal);
        return javaBitmap;
    }

    BitmapUtils::BitmapUtils() {
    }

}
