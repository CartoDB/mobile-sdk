/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ASSETPACKAGE_H_
#define _CARTO_ASSETPACKAGE_H_

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace carto {
    class BinaryData;

    /**
     * An abstract asset package class. Contains methods to list and load assets.
     */
    class AssetPackage {
    public:
        virtual ~AssetPackage() { }
    
        /**
         * Returns list of assets in the asset package.
         * All listed assets can read using loadAsset API.
         * @return List of asset names.
         */
        virtual std::vector<std::string> getAssetNames() const = 0;
    
        /**
         * Loads the specified asset from the asset package.
         * @param name The asset name to read.
         * @returns Asset data, if the asset exists and was successfully read. Null pointer otherwise.
         */
        virtual std::shared_ptr<BinaryData> loadAsset(const std::string& name) const = 0;

    protected:
        AssetPackage() { }
    };
    
}

#endif
