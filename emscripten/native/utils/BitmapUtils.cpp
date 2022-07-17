#include "BitmapUtils.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "graphics/Bitmap.h"
#include "utils/AssetUtils.h"
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
        if (size <= 0) {
            Log::Errorf("BitmapUtils::LoadBitmapFromFile: Ignore load due to size %d: %s", size, filePath.c_str());
            return std::shared_ptr<Bitmap>();
        }
        std::vector<unsigned char> data(static_cast<std::size_t>(size));
        fread(data.data(), 1, size, fp.get());
        return Bitmap::CreateFromCompressed(data.data(), data.size());
    }


    BitmapUtils::BitmapUtils() {
    }

}
