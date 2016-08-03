#include "SolidLayer.h"
#include "components/Exceptions.h"
#include "graphics/Bitmap.h"
#include "renderers/SolidRenderer.cpp"
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
    }

    SolidLayer::~SolidLayer() {
    }
        
    Color SolidLayer::getColor() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _color;
    }
    
    void SolidLayer::setColor(const Color& color) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _color = color;
    }

    std::shared_ptr<Bitmap> SolidLayer::getBitmap() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _bitmap;
    }

    void SolidLayer::setBitmap(const std::shared_ptr<Bitmap>& bitmap) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _bitmap = bitmap;
    }

    float SolidLayer::getBitmapScale() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _bitmapScale;
    }

    void SolidLayer::setBitmapScale(float scale) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _bitmapScale = scale;
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
    }
    
    void SolidLayer::loadData(const std::shared_ptr<CullState>& cullState) {
    }

    void SolidLayer::offsetLayerHorizontally(double offset) {
    }
    
    void SolidLayer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        Layer::onSurfaceCreated(shaderManager, textureManager);
        _solidRenderer->onSurfaceCreated(shaderManager, textureManager);
    }
    
    bool SolidLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter,
                                  StyleTextureCache& styleCache,
                                  const ViewState& viewState)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _solidRenderer->setColor(_color);
            _solidRenderer->setBitmap(_bitmap, _bitmapScale);
        }
        _solidRenderer->onDrawFrame(viewState);
        return false;
    }
    
    void SolidLayer::onSurfaceDestroyed(){
        _solidRenderer->onSurfaceDestroyed();
        Layer::onSurfaceDestroyed();
    }
    
    void SolidLayer::calculateRayIntersectedElements(const Projection& projection, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
    }

    bool SolidLayer::processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const {
        return clickType == ClickType::CLICK_TYPE_SINGLE || clickType == ClickType::CLICK_TYPE_LONG; // by default, disable 'click through' for single and long clicks
    }
    
}
