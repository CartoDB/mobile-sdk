#include "MemoryAssetPackage.h"
#include "core/BinaryData.h"

namespace carto {

    MemoryAssetPackage::MemoryAssetPackage(const std::map<std::string, std::shared_ptr<BinaryData> >& localAssets) :
        _localAssets(localAssets),
        _baseAssetPackage()
    {
    }

    MemoryAssetPackage::MemoryAssetPackage(const std::map<std::string, std::shared_ptr<BinaryData> >& localAssets, const std::shared_ptr<AssetPackage>& baseAssetPackage) :
        _localAssets(localAssets),
        _baseAssetPackage(baseAssetPackage)
    {
    }

    MemoryAssetPackage::~MemoryAssetPackage() {
    }

    std::vector<std::string> MemoryAssetPackage::getLocalAssetNames() const {
        std::vector<std::string> assetNames;
        for (auto it = _localAssets.begin(); it != _localAssets.end(); it++) {
            assetNames.push_back(it->first);
        }
        return assetNames;
    }

    std::vector<std::string> MemoryAssetPackage::getAssetNames() const {
        std::vector<std::string> assetNames;
        if (_baseAssetPackage) {
            assetNames = _baseAssetPackage->getAssetNames();
        }

        for (auto it = _localAssets.begin(); it != _localAssets.end(); it++) {
            auto oldIt = std::find(assetNames.begin(), assetNames.end(), it->first);
            if (oldIt != assetNames.end()) {
                oldIt = assetNames.erase(oldIt);
            }
            if (it->second) {
                assetNames.insert(oldIt, it->first);
            }
        }

        return assetNames;
    }

    std::shared_ptr<BinaryData> MemoryAssetPackage::loadAsset(const std::string& name) const {
        auto it = _localAssets.find(name);
        if (it != _localAssets.end()) {
            return it->second;
        }

        if (_baseAssetPackage) {
            return _baseAssetPackage->loadAsset(name);
        }
        return std::shared_ptr<BinaryData>();
    }

}
