#include "CartoVectorTileLayer.h"
#include "assets/CartoStylesV1ZIP.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "styles/CompiledStyleSet.h"
#include "vectortiles/MBVectorTileDecoder.h"
#include "utils/AssetPackage.h"
#include "utils/ZippedAssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CartoVectorTileLayer::CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, CartoBaseMapStyle::CartoBaseMapStyle style) :
        VectorTileLayer(dataSource, CreateTileDecoder(style))
    {
    }

    CartoVectorTileLayer::CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        VectorTileLayer(dataSource, CreateTileDecoder(styleAssetPackage))
    {
    }
    
    CartoVectorTileLayer::~CartoVectorTileLayer() {
    }

    std::string CartoVectorTileLayer::getLanguage() const {
        auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder());
        return tileDecoder->getStyleParameter("lang");
    }

    void CartoVectorTileLayer::setLanguage(const std::string& lang) {
        auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder());
        tileDecoder->setStyleParameter("lang", lang);
    }
    
    std::shared_ptr<VectorTileDecoder> CartoVectorTileLayer::CreateTileDecoder(CartoBaseMapStyle::CartoBaseMapStyle style) {
        auto styleAsset = std::make_shared<BinaryData>(cartostyles_v1_zip, cartostyles_v1_zip_len);
        std::string styleName;
        switch (style) {
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_VOYAGER:
            styleName = "voyager";
            break;
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_POSITRON:
            styleName = "positron";
            break;
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_DARKMATTER:
            styleName = "darkmatter";
            break;
        default:
            styleName = "voyager";
            break;
        }
        auto styleAssetPackage = std::make_shared<ZippedAssetPackage>(styleAsset);
        return std::make_shared<MBVectorTileDecoder>(std::make_shared<CompiledStyleSet>(styleAssetPackage, styleName));
    }

    std::shared_ptr<VectorTileDecoder> CartoVectorTileLayer::CreateTileDecoder(const std::shared_ptr<AssetPackage>& styleAssetPackage) {
        if (!styleAssetPackage) {
            throw NullArgumentException("Null styleAssetPackage");
        }
        return std::make_shared<MBVectorTileDecoder>(std::make_shared<CompiledStyleSet>(styleAssetPackage));
    }
    
}
