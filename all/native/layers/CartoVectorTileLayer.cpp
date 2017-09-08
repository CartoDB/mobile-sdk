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
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder())) {
            return tileDecoder->getStyleParameter("lang");
        }
        return std::string();
    }

    void CartoVectorTileLayer::setLanguage(const std::string& lang) {
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder())) {
            tileDecoder->setStyleParameter("lang", lang);
        }
    }
    
    std::shared_ptr<VectorTileDecoder> CartoVectorTileLayer::CreateTileDecoder(CartoBaseMapStyle::CartoBaseMapStyle style) {
        return std::make_shared<MBVectorTileDecoder>(std::make_shared<CompiledStyleSet>(CreateStyleAssetPackage(), GetStyleName(style)));
    }

    std::shared_ptr<VectorTileDecoder> CartoVectorTileLayer::CreateTileDecoder(const std::shared_ptr<AssetPackage>& styleAssetPackage) {
        if (!styleAssetPackage) {
            throw NullArgumentException("Null styleAssetPackage");
        }
        return std::make_shared<MBVectorTileDecoder>(std::make_shared<CompiledStyleSet>(styleAssetPackage));
    }

    std::shared_ptr<AssetPackage> CartoVectorTileLayer::CreateStyleAssetPackage() {
        auto styleAsset = std::make_shared<BinaryData>(cartostyles_v1_zip, cartostyles_v1_zip_len);
        return std::make_shared<ZippedAssetPackage>(styleAsset);
    }

    std::string CartoVectorTileLayer::GetStyleName(CartoBaseMapStyle::CartoBaseMapStyle style) {
        switch (style) {
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_VOYAGER:
            return "voyager";
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_POSITRON:
            return "positron";
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_DARKMATTER:
            return "darkmatter";
        default:
            return "voyager";
        }
    }

    std::string CartoVectorTileLayer::GetStyleSource(CartoBaseMapStyle::CartoBaseMapStyle style) {
        switch (style) {
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_VOYAGER:
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_POSITRON:
        case CartoBaseMapStyle::CARTO_BASEMAP_STYLE_DARKMATTER:
            return "carto.streets";
        default:
            return "carto.streets";
        }
    }

}
