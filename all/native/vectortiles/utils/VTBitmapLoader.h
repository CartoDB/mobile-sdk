/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VTBITMAPLOADER_H_
#define _CARTO_VTBITMAPLOADER_H_

#include "core/BinaryData.h"
#include "graphics/Bitmap.h"
#include "network/URLFileLoader.h"
#include "utils/AssetPackage.h"
#include "utils/FileUtils.h"
#include "utils/Log.h"

#include <vector>

#include <vt/Bitmap.h>
#include <vt/BitmapManager.h>

namespace carto {
    
    class VTBitmapLoader : public vt::BitmapManager::BitmapLoader {
    public:
        VTBitmapLoader(const std::string& basePath, const std::shared_ptr<AssetPackage>& assetPackage) :  _basePath(basePath), _assetPackage(assetPackage), _urlFileLoader("VTBitmapLoader", true) { }
    
        virtual std::shared_ptr<vt::Bitmap> load(const std::string& url) const {
            std::shared_ptr<BinaryData> fileData;
            if (!_urlFileLoader.loadFile(url, fileData)) {
                std::string fileName = FileUtils::NormalizePath(_basePath + url);
                if (_assetPackage) {
                    fileData = _assetPackage->loadAsset(fileName);
                }
                if (!fileData) {
                    Log::Errorf("VTBitmapLoader: Failed to load bitmap: %s", fileName.c_str());
                }
            }
            if (!fileData) {
                return std::shared_ptr<vt::Bitmap>();
            }
            
            std::shared_ptr<Bitmap> sourceBitmap = Bitmap::CreateFromCompressed(fileData->data(), fileData->size());
            if (!sourceBitmap) {
                Log::Errorf("VTBitmapLoader: Failed to decode bitmap: %s", url.c_str());
                return std::shared_ptr<vt::Bitmap>();                
            }
            sourceBitmap = sourceBitmap->getRGBABitmap();
            std::vector<std::uint32_t> data(sourceBitmap->getWidth() * sourceBitmap->getHeight());
            for (unsigned int y = 0; y < sourceBitmap->getHeight(); y++) {
                int dataOffset = (sourceBitmap->getHeight() - 1 - y) * sourceBitmap->getWidth();
                int bitmapOffset = y * sourceBitmap->getWidth() * sourceBitmap->getBytesPerPixel();
                for (unsigned int x = 0; x < sourceBitmap->getWidth(); x++) {
                    data[dataOffset++] = *reinterpret_cast<const std::uint32_t*>(&sourceBitmap->getPixelData()[bitmapOffset]);
                    bitmapOffset += 4;
                }
            }
            return std::make_shared<vt::Bitmap>(sourceBitmap->getWidth(), sourceBitmap->getHeight(), std::move(data));
        }
    
    private:
        std::string _basePath;
        std::shared_ptr<AssetPackage> _assetPackage;
        URLFileLoader _urlFileLoader;
    };
    
}

#endif
