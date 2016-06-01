/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ASSETUTILS_H_
#define _CARTO_ASSETUTILS_H_

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
         * Loads the specified bundled resource.
         * @param path The path of the resource to load. The path is relative to application root folder.
         * @return The loaded resource as a byte vector or null if the resource was not found or could not be loaded.
         */
        static std::shared_ptr<BinaryData> LoadAsset(const std::string& path);

        /**
         * Calculates path for the bundled resource.
         * @param resourceName The name of the resource.
         * @return The full path for the resource. Result will be empty string if the resource was not found.
         */
        static std::string CalculateResourcePath(const std::string& resourceName);
        
        /**
         * Calculates writable path for the given file name.
         * @param fileName The file name to use.
         * @return The full path for the given file name.
         */
        static std::string CalculateWritablePath(const std::string& fileName);

    private:
        AssetUtils();
    };

}

#endif
