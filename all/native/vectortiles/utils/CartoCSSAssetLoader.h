/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSSASSETLOADER_H_
#define _CARTO_CARTOCSSASSETLOADER_H_

#include "utils/URLFileLoader.h"

#include <string>
#include <memory>

#include <cartocss/CartoCSSMapLoader.h>

namespace carto {
    class AssetPackage;
    
    class CartoCSSAssetLoader : public css::CartoCSSMapLoader::AssetLoader {
    public:
        CartoCSSAssetLoader(const std::string& basePath, const std::shared_ptr<AssetPackage>& assetPackage);
        virtual ~CartoCSSAssetLoader();
        
        virtual std::shared_ptr<const std::vector<unsigned char> > load(const std::string& url) const;    

    private:
        std::string _basePath;
        std::shared_ptr<AssetPackage> _assetPackage;
        URLFileLoader _urlFileLoader;
    };
    
}

#endif
