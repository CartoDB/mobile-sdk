/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ASSETUTILS_H_
#define _CARTO_ASSETUTILS_H_

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <jni.h>
#include <android/asset_manager.h>

namespace carto {
    class BinaryData;

    /**
     * A helper class for managing application-bundled assets.
     */
    class AssetUtils {
    public:
        /**
         * Internal method for connecting to asset manager.
         * @param androidAssetManager The asset manager instance to use.
         */
        static void SetAssetManagerPointer(jobject androidAssetManager);

        /**
         * Loads the specified bundled asset.
         * @param path The path of the asset to load. The path is relative to application root folder.
         * @return The loaded asset as a byte vector or null if the asset was not found or could not be loaded.
         */
        static std::shared_ptr<BinaryData> LoadAsset(const std::string& path);

    private:
        AssetUtils();

        static AAssetManager* _AssetManagerPtr;
        static std::mutex _Mutex;
    };

}

#endif
