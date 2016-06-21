/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSSASSETLOADER_H_
#define _CARTO_CARTOCSSASSETLOADER_H_

#include "network/URLFileLoader.h"
#include "utils/FileUtils.h"
#include "utils/Log.h"
#include "utils/AssetPackage.h"

#include <cartocss/CartoCSSMapLoader.h>

namespace carto {
    
    class CartoCSSAssetLoader : public css::CartoCSSMapLoader::AssetLoader {
    public:
        CartoCSSAssetLoader(const std::string& basePath, const std::shared_ptr<AssetPackage>& assetPackage) : _basePath(basePath), _assetPackage(assetPackage), _urlFileLoader("CartoCSSAssetLoader", true) { }
        
        virtual std::shared_ptr<std::vector<unsigned char> > load(const std::string& url) const {
            std::shared_ptr<BinaryData> data;
            if (!_urlFileLoader.loadFile(url, data)) {
                std::string fileName = FileUtils::NormalizePath(_basePath + url);
                if (_assetPackage) {
                    data = _assetPackage->loadAsset(fileName);
                }
                if (!data) {
                    Log::Errorf("CartoCSSAssetLoader: Failed to load asset: %s", fileName.c_str());
                }
            }
            return data ? data->getDataPtr() : std::shared_ptr<std::vector<unsigned char> >();
        }
    
    private:
        std::string _basePath;
        std::shared_ptr<AssetPackage> _assetPackage;
        URLFileLoader _urlFileLoader;
    };
    
}

#endif
