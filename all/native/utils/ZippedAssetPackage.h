/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ZIPPEDASSETPACKAGE_H_
#define _CARTO_ZIPPEDASSETPACKAGE_H_

#include "utils/AssetPackage.h"

#include <mutex>

namespace carto {

    /**
     * An asset package based on ZIP archived.
     * Only deflate-based ZIP archives are supported.
     */
    class ZippedAssetPackage : public AssetPackage {
    public:
        /**
         * Constructs a ZIP asset package from the archived binary data.
         * @param zipData The ZIP archive.
         */
        explicit ZippedAssetPackage(const std::shared_ptr<BinaryData>& zipData);
        /**
         * Constructs a ZIP asset package from the archived binary data and a fallback asset package.
         * @param zipData The ZIP archive.
         * @param baseAssetPackage The base asset package. If an asset is not found in the ZIP archive, base asset package is used.
         */
        ZippedAssetPackage(const std::shared_ptr<BinaryData>& zipData, const std::shared_ptr<AssetPackage>& baseAssetPackage);
        virtual ~ZippedAssetPackage();

        std::vector<std::string> getLocalAssetNames() const;
    
        virtual std::vector<std::string> getAssetNames() const;
    
        virtual std::shared_ptr<BinaryData> loadAsset(const std::string& name) const;
    
    private:
        void initialize();
        void deinitialize();

        const std::shared_ptr<BinaryData> _zipData;
        const std::shared_ptr<AssetPackage> _baseAssetPackage;
        std::shared_ptr<void> _handle;
        std::map<std::string, unsigned int> _assetIndexMap;

        mutable std::mutex _mutex;
    };
    
}

#endif
