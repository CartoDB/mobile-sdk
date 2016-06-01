/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ASSETUTILS_H_
#define _CARTO_ASSETUTILS_H_

#include <memory>
#include <string>
#include <vector>

namespace carto {
    class BinaryData;

    /**
     * A helper class for managing application-bundled assets.
     */
    class AssetUtils {
    public:

        /**
         * Loads the specified bundled asset.
         * @param path The path of the asset to load. The path is relative to the asset in 'Assets' folder.
         * @return The loaded asset as a byte vector or null if the asset was not found or could not be loaded.
         */
        static std::shared_ptr<BinaryData> LoadAsset(const std::string& path);

    private:
        AssetUtils();
    };

}

#endif
