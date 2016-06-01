#include "CartoCSSStyleSet.h"
#include "core/BinaryData.h"
#include "vectortiles/AssetPackage.h"

namespace carto {
    
    CartoCSSStyleSet::CartoCSSStyleSet(const std::string& cartoCSS) :
        _cartoCSS(cartoCSS)
    {
    }

    CartoCSSStyleSet::CartoCSSStyleSet(const std::string& cartoCSS, const std::shared_ptr<AssetPackage>& assetPackage) :
        _cartoCSS(cartoCSS), _assetPackage(assetPackage)
    {
    }

    CartoCSSStyleSet::~CartoCSSStyleSet() {
    }

    const std::string& CartoCSSStyleSet::getCartoCSS() const {
         return _cartoCSS;
    }

    const std::shared_ptr<AssetPackage>& CartoCSSStyleSet::getAssetPackage() const {
         return _assetPackage;
    }

}
