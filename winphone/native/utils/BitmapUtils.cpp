#include "BitmapUtils.h"
#include "core/BinaryData.h"
#include "utils/AssetUtils.h"
#include "graphics/Bitmap.h"
#include "graphics/utils/GLContext.h"
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

    BitmapUtils::BitmapUtils() {
    }

}
