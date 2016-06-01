#include "AssetUtils.h"
#include "core/BinaryData.h"
#include "utils/AndroidUtils.h"
#include "utils/Log.h"

#include <android/asset_manager_jni.h>

namespace carto {

    void AssetUtils::SetAssetManagerPointer(jobject androidAssetManager) {
        std::lock_guard<std::mutex> lock(_Mutex);
        _AssetManagerPtr = AAssetManager_fromJava(AndroidUtils::GetCurrentThreadJNIEnv(), androidAssetManager);
    }

    std::shared_ptr<BinaryData> AssetUtils::LoadAsset(const std::string& path) {
        std::shared_ptr<AAsset> asset;
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            if (!_AssetManagerPtr) {
                Log::Error("AssetManager::LoadAsset: Asset manager pointer not set yet");
                return std::shared_ptr<BinaryData>();
            }

            asset = std::shared_ptr<AAsset>(AAssetManager_open(_AssetManagerPtr, path.c_str(), AASSET_MODE_UNKNOWN), AAsset_close);
        }

        if (!asset) {
            Log::Errorf("AssetManager::LoadAsset: Asset not found: %s", path.c_str());
            return std::shared_ptr<BinaryData>();
        }

        int size = AAsset_getLength(asset.get());
        if (size < 0) {
            Log::Errorf("AssetManager::LoadAsset: Asset size is <0: %s", path.c_str());
            return std::shared_ptr<BinaryData>();
        }
        std::vector<unsigned char> data(size);

        AAsset_read(asset.get(), data.data(), size);

        return std::make_shared<BinaryData>(std::move(data));
    }

    AssetUtils::AssetUtils() {
    }

    AAssetManager* AssetUtils::_AssetManagerPtr = nullptr;
    std::mutex AssetUtils::_Mutex;

}
