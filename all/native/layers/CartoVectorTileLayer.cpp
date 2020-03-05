#include "CartoVectorTileLayer.h"
#include "assets/CartoStylesV1ZIP.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "styles/CompiledStyleSet.h"
#include "vectortiles/MBVectorTileDecoder.h"
#include "utils/AssetPackage.h"
#include "utils/ZippedAssetPackage.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>

namespace carto {
    
    CartoVectorTileLayer::CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, CartoBaseMapStyle::CartoBaseMapStyle style) :
        VectorTileLayer(dataSource, CreateTileDecoder(style))
    {
    }

    CartoVectorTileLayer::CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        VectorTileLayer(dataSource, CreateTileDecoder(styleAssetPackage))
    {
    }
    
    CartoVectorTileLayer::CartoVectorTileLayer(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<AssetPackage>& styleAssetPackage, const std::string& styleName) :
        VectorTileLayer(dataSource, CreateTileDecoder(styleAssetPackage, styleName))
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
    
    std::string CartoVectorTileLayer::getFallbackLanguage() const {
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder())) {
            return tileDecoder->getStyleParameter("fallback_lang");
        }
        return std::string();
    }

    void CartoVectorTileLayer::setFallbackLanguage(const std::string& lang) {
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder())) {
            tileDecoder->setStyleParameter("fallback_lang", lang);
        }
    }

    CartoBaseMapPOIRenderMode::CartoBaseMapPOIRenderMode CartoVectorTileLayer::getPOIRenderMode() const {
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder())) {
            return static_cast<CartoBaseMapPOIRenderMode::CartoBaseMapPOIRenderMode>(boost::lexical_cast<int>(tileDecoder->getStyleParameter("icons")));
        }
        return CartoBaseMapPOIRenderMode::CARTO_BASE_MAP_POI_RENDER_MODE_NONE;
    }

    void CartoVectorTileLayer::setPOIRenderMode(CartoBaseMapPOIRenderMode::CartoBaseMapPOIRenderMode renderMode) {
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder())) {
            tileDecoder->setStyleParameter("pois", boost::lexical_cast<std::string>(static_cast<int>(renderMode)));
        }
    }
    
    CartoBaseMapBuildingRenderMode::CartoBaseMapBuildingRenderMode CartoVectorTileLayer::getBuildingRenderMode() const {
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder())) {
            return static_cast<CartoBaseMapBuildingRenderMode::CartoBaseMapBuildingRenderMode>(boost::lexical_cast<int>(tileDecoder->getStyleParameter("pois")));
        }
        return CartoBaseMapBuildingRenderMode::CARTO_BASE_MAP_BUILDING_RENDER_MODE_NONE;
    }

    void CartoVectorTileLayer::setBuildingRenderMode(CartoBaseMapBuildingRenderMode::CartoBaseMapBuildingRenderMode renderMode) {
        if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(getTileDecoder())) {
            tileDecoder->setStyleParameter("buildings", boost::lexical_cast<std::string>(static_cast<int>(renderMode)));
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

    std::shared_ptr<VectorTileDecoder> CartoVectorTileLayer::CreateTileDecoder(const std::shared_ptr<AssetPackage>& styleAssetPackage, const std::string& styleName) {
        if (!styleAssetPackage) {
            throw NullArgumentException("Null styleAssetPackage");
        }
        return std::make_shared<MBVectorTileDecoder>(std::make_shared<CompiledStyleSet>(styleAssetPackage, styleName));
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
