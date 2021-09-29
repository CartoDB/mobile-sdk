#include "CartoOnlineVectorTileLayer.h"
#include "datasources/CartoOnlineTileDataSource.h"
#include "styles/CompiledStyleSet.h"
#include "vectortiles/utils/CartoAssetPackageUpdater.h"
#include "vectortiles/MBVectorTileDecoder.h"
#include "utils/MemoryAssetPackage.h"
#include "utils/Log.h"

namespace carto {
    
    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(CartoBaseMapStyle::CartoBaseMapStyle style) :
        CartoVectorTileLayer(CreateDataSource(style), style),
        _style(style),
        _styleUpdateThreadPool(std::make_shared<CancelableThreadPool>())
    {
        _styleUpdateThreadPool->setPoolSize(1);
    }

    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string& source, CartoBaseMapStyle::CartoBaseMapStyle style) :
        CartoVectorTileLayer(std::make_shared<CartoOnlineTileDataSource>(source), style),
        _style(style),
        _styleUpdateThreadPool(std::make_shared<CancelableThreadPool>())
    {
        _styleUpdateThreadPool->setPoolSize(1);
    }

    CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string& source, const std::shared_ptr<AssetPackage>& styleAssetPackage) :
        CartoVectorTileLayer(std::make_shared<CartoOnlineTileDataSource>(source), styleAssetPackage),
        _style(),
        _styleUpdateThreadPool()
    {
    }
    
    CartoOnlineVectorTileLayer::~CartoOnlineVectorTileLayer() {
        if (_styleUpdateThreadPool) {
            _styleUpdateThreadPool->cancelAll();
            _styleUpdateThreadPool->deinit();
        }
    }

    std::shared_ptr<TileDataSource> CartoOnlineVectorTileLayer::CreateDataSource(CartoBaseMapStyle::CartoBaseMapStyle style) {
        return std::make_shared<CartoOnlineTileDataSource>(GetStyleSource(style));
    }

    void CartoOnlineVectorTileLayer::setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
        const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
        const std::weak_ptr<Options>& options,
        const std::weak_ptr<MapRenderer>& mapRenderer,
        const std::weak_ptr<TouchHandler>& touchHandler)
    {
        CartoVectorTileLayer::setComponents(envelopeThreadPool, tileThreadPool, options, mapRenderer, touchHandler);
        if (envelopeThreadPool && tileThreadPool && _styleUpdateThreadPool) {
            _styleUpdateThreadPool->execute(std::make_shared<StyleUpdateTask>(std::static_pointer_cast<CartoOnlineVectorTileLayer>(shared_from_this()), _style));
        }
    }

    CartoOnlineVectorTileLayer::StyleUpdateTask::StyleUpdateTask(const std::shared_ptr<CartoOnlineVectorTileLayer>& layer, CartoBaseMapStyle::CartoBaseMapStyle style) :
        _layer(layer),
        _style(style)
    {
    }

    void CartoOnlineVectorTileLayer::StyleUpdateTask::run() {
        std::string schema;
        std::string styleName = GetStyleName(_style);
        std::shared_ptr<AssetPackage> currentAssetPackage;
        {
            if (auto layer = _layer.lock()) {
                if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(layer->getTileDecoder())) {
                    if (auto compiledStyleSet = tileDecoder->getCompiledStyleSet()) {
                        currentAssetPackage = compiledStyleSet->getAssetPackage();
                    }
                }

                if (auto dataSource = std::dynamic_pointer_cast<CartoOnlineTileDataSource>(layer->getDataSource())) {
                    schema = dataSource->getSchema();
                }
            }
        }
        if (schema.empty()) {
            schema = GetStyleSource(_style) + "/v2"; // default schema, if missing
        }

        std::shared_ptr<MemoryAssetPackage> newAssetPackage;
        try {
            CartoAssetPackageUpdater updater(schema, styleName);
            newAssetPackage = updater.update(currentAssetPackage);
        }
        catch (const std::exception& ex) {
            Log::Errorf("CartoOnlineVectorTileLayer::StyleUpdateTask: Error while updating style: %s", ex.what());
            return;
        }

        if (newAssetPackage && !newAssetPackage->getLocalAssetNames().empty()) {
            if (auto layer = _layer.lock()) {
                if (auto tileDecoder = std::dynamic_pointer_cast<MBVectorTileDecoder>(layer->getTileDecoder())) {
                    tileDecoder->setCompiledStyleSet(std::make_shared<CompiledStyleSet>(newAssetPackage, styleName));
                }
            }
        }
    }
    
}
