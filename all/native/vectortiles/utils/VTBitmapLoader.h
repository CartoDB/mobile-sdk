/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VTBITMAPLOADER_H_
#define _CARTO_VTBITMAPLOADER_H_

#include "utils/URLFileLoader.h"

#include <string>
#include <vector>
#include <memory>

#include <vt/Bitmap.h>
#include <vt/BitmapManager.h>

namespace carto {
    class AssetPackage;
    
    class VTBitmapLoader : public vt::BitmapManager::BitmapLoader {
    public:
        VTBitmapLoader(const std::string& basePath, const std::shared_ptr<AssetPackage>& assetPackage);
        virtual ~VTBitmapLoader();
    
        virtual std::shared_ptr<const vt::Bitmap> load(const std::string& url, float& resolution) const;
    
    private:
        std::shared_ptr<const vt::Bitmap> loadSVG(const std::vector<unsigned char>& fileData, float& resolution) const;

        std::string _basePath;
        std::shared_ptr<AssetPackage> _assetPackage;
        URLFileLoader _urlFileLoader;
    };
    
}

#endif
