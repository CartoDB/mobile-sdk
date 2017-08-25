#include "CartoCSSAssetLoader.h"
#include "core/BinaryData.h"
#include "utils/FileUtils.h"
#include "utils/Log.h"
#include "utils/AssetPackage.h"

namespace carto {
    
    CartoCSSAssetLoader::CartoCSSAssetLoader(const std::string& basePath, const std::shared_ptr<AssetPackage>& assetPackage) :
        _basePath(basePath),
        _assetPackage(assetPackage),
        _urlFileLoader("CartoCSSAssetLoader", true)
    {
    }

    CartoCSSAssetLoader::~CartoCSSAssetLoader() {
    }
        
    std::shared_ptr<std::vector<unsigned char> > CartoCSSAssetLoader::load(const std::string& url) const {
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

        if (!data) {
            return std::shared_ptr<std::vector<unsigned char> >();
        }
        return data->getDataPtr();
    }
    
}
