#include "CartoCSSAssetLoader.h"
#include "core/BinaryData.h"
#include "utils/FileUtils.h"
#include "utils/Log.h"
#include "utils/AssetPackage.h"

namespace carto {
    
    CartoCSSAssetLoader::CartoCSSAssetLoader(const std::string& basePath, const std::shared_ptr<AssetPackage>& assetPackage) :
        _basePath(basePath),
        _assetPackage(assetPackage),
        _urlFileLoader()
    {
        _urlFileLoader.setCaching(true);
    }

    CartoCSSAssetLoader::~CartoCSSAssetLoader() {
    }
        
    std::shared_ptr<const std::vector<unsigned char> > CartoCSSAssetLoader::load(const std::string& url) const {
        std::shared_ptr<BinaryData> fileData;
        if (_urlFileLoader.isSupported(url)) {
            if (!_urlFileLoader.load(url, fileData)) {
                Log::Errorf("CartoCSSAssetLoader: Failed to load asset: %s", url.c_str());
            }
        } else {
            std::string fileName = FileUtils::NormalizePath(_basePath + url);
            if (_assetPackage) {
                fileData = _assetPackage->loadAsset(fileName);
            }
            if (!fileData) {
                Log::Errorf("CartoCSSAssetLoader: Failed to load asset from asset package: %s", fileName.c_str());
            }
        }

        if (!fileData) {
            return std::shared_ptr<std::vector<unsigned char> >();
        }
        return fileData->getDataPtr();
    }
    
}
