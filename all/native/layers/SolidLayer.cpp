#include "SolidLayer.h"
#include "components/Exceptions.h"
#include "graphics/Bitmap.h"
#include "renderers/SolidRenderer.h"
#include "utils/Log.h"

#include <vector>

namespace carto {

    SolidLayer::SolidLayer(const Color& color) :
        Layer(),
        _color(color),
        _bitmap(),
        _bitmapScale(1.0f),
        _solidRenderer(std::make_shared<SolidRenderer>())
    {
    }
    
    SolidLayer::SolidLayer(const std::shared_ptr<Bitmap>& bitmap) :
        Layer(),
        _color(Color(255, 255, 255, 255)),
        _bitmap(bitmap),
        _bitmapScale(1.0f),
        _solidRenderer(std::make_shared<SolidRenderer>())
    {
        if (!bitmap) {
            throw NullArgumentException("Null bitmap");
        }
    }

    SolidLayer::~SolidLayer() {
    }
        
    Color SolidLayer::getColor() const {
        return _color.load();
    }
    
    void SolidLayer::setColor(const Color& color) {
        _color.store(color);
        redraw();
    }

    std::shared_ptr<Bitmap> SolidLayer::getBitmap() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _bitmap;
    }

    void SolidLayer::setBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _bitmap = bitmap;
        }
        redraw();
    }

    float SolidLayer::getBitmapScale() const {
        return _bitmapScale.load();
    }

    void SolidLayer::setBitmapScale(float scale) {
        _bitmapScale.store(scale);
        redraw();
    }

    bool SolidLayer::isUpdateInProgress() const {
        return false;
    }

    void SolidLayer::setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                    const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                    const std::weak_ptr<Options>& options,
                                    const std::weak_ptr<MapRenderer>& mapRenderer,
                                    const std::weak_ptr<TouchHandler>& touchHandler)
    {
        Layer::setComponents(envelopeThreadPool, tileThreadPool, options, mapRenderer, touchHandler);
        _solidRenderer->setComponents(options, mapRenderer);
    }
    
    void SolidLayer::loadData(const std::shared_ptr<CullState>& cullState) {
    }

    void SolidLayer::offsetLayerHorizontally(double offset) {
    }
    
    bool SolidLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState) {
        Color color = getColor();
        _solidRenderer->setColor(Color(color.getR(), color.getG(), color.getB(), static_cast<unsigned char>(color.getA() * getOpacity())));
        _solidRenderer->setBitmap(getBitmap(), getBitmapScale());
        _solidRenderer->onDrawFrame(viewState);
        return false;
    }
    
    std::shared_ptr<Bitmap> SolidLayer::getBackgroundBitmap() const {
        return std::shared_ptr<Bitmap>();
    }
    
    std::shared_ptr<Bitmap> SolidLayer::getSkyBitmap() const {
        return std::shared_ptr<Bitmap>();
    }
    
    void SolidLayer::calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
    }

    bool SolidLayer::processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const {
        return clickType == ClickType::CLICK_TYPE_SINGLE || clickType == ClickType::CLICK_TYPE_LONG; // by default, disable 'click through' for single and long clicks
    }
    
    void SolidLayer::registerDataSourceListener() {
    }

    void SolidLayer::unregisterDataSourceListener() {
    }

}
