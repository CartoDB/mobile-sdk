#include "CompiledStyleSet.h"
#include "vectortiles/AssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CompiledStyleSet::CompiledStyleSet(const std::shared_ptr<AssetPackage>& assetPackage) :
        _styleName(), _styleAssetName(), _assetPackage(assetPackage)
    {
        // Find first .xml or .json from the root directory, use this as the default style
        for (const std::string& assetName : _assetPackage->getAssetNames()) {
            std::string::size_type dirPos = assetName.find('/');
            std::string::size_type extPos = assetName.rfind('.');
            if (dirPos == std::string::npos && extPos != std::string::npos) {
                std::string ext = assetName.substr(extPos);
                std::string assetNameWithoutExt = assetName.substr(0, extPos);
                if (ext == ".xml" || ext == ".json") {
                    if (_styleAssetName.empty() || assetNameWithoutExt < _styleName) {
                        _styleName = assetNameWithoutExt;
                        _styleAssetName = assetName;
                    }
                }
            }
        }
        if (_styleAssetName.empty()) {
            Log::Error("CompiledStyleSet::CompiledStyleSet: Could not find any styles in the style set");
        }
    }

    CompiledStyleSet::CompiledStyleSet(const std::shared_ptr<AssetPackage>& assetPackage, const std::string& styleName) :
        _styleName(), _styleAssetName(), _assetPackage(assetPackage)
    {
        for (const std::string& assetName : _assetPackage->getAssetNames()) {
            if (assetName == styleName + ".xml" || assetName == styleName + ".json") {
                _styleName = styleName;
                _styleAssetName = assetName;
            }
        }
        if (_styleAssetName.empty()) {
            Log::Errorf("CompiledStyleSet::CompiledStyleSet: Could not find specified style in the style set: %s", styleName.c_str());
        }
    }

    CompiledStyleSet::~CompiledStyleSet() {
    }

    const std::string& CompiledStyleSet::getStyleName() const {
         return _styleName;
    }

    const std::string& CompiledStyleSet::getStyleAssetName() const {
         return _styleAssetName;
    }

    const std::shared_ptr<AssetPackage>& CompiledStyleSet::getAssetPackage() const {
         return _assetPackage;
    }

}
