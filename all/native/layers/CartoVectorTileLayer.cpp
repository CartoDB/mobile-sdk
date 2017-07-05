#include "CartoVectorTileLayer.h"
#include "assets/NutibrightV3ZIP.h"
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
        std::shared_ptr<BinaryData> styleAsset;
        std::string styleName;
        switch (style) {
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_DARK:
            styleAsset = std::make_shared<BinaryData>(nutibright_v3_zip, nutibright_v3_zip_len);
            styleName = "nutiteq_dark";
            break;
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_GRAY:
            styleAsset = std::make_shared<BinaryData>(nutibright_v3_zip, nutibright_v3_zip_len);
            styleName = "nutiteq_grey";
            break;
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_VOYAGER:
            styleAsset = std::make_shared<BinaryData>(cartostyles_v1_zip, cartostyles_v1_len);
            styleName = "voyager";
            break;
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_POSITRON:
            styleAsset = std::make_shared<BinaryData>(cartostyles_v1_zip, cartostyles_v1_len);
            styleName = "positron";
            break;
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_DARKMATTER:
            styleAsset = std::make_shared<BinaryData>(cartostyles_v1_zip, cartostyles_v1_len);
            styleName = "darkmatter";
            break;
        default:
            styleAsset = std::make_shared<BinaryData>(nutibright_v3_zip, nutibright_v3_zip_len);
            styleName = "style";
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
