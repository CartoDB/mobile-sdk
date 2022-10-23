#ifndef _CARTO_ASSETUTILS_H_
#define _CARTO_ASSETUTILS_H_

#include <memory>
#include <mutex>
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
         * @param path The path of the asset to load. The path is relative to application root folder.
         * @return The loaded asset as a byte vector or null if the asset was not found or could not be loaded.
         */
        static std::shared_ptr<BinaryData> LoadAsset(const std::string& path);

    private:
        AssetUtils();
        static std::mutex _Mutex;
    };

}

#endif
