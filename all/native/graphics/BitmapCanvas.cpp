#include "BitmapCanvas.h"

#if defined(_WIN32)
#define CARTO_BITMAP_CANVAS_IMPL UWPImpl
#include "graphics/BitmapCanvasUWPImpl.h"
#elif defined(__APPLE__)
#define CARTO_BITMAP_CANVAS_IMPL IOSImpl
#include "graphics/BitmapCanvasIOSImpl.h"
#elif defined(__ANDROID__)
#define CARTO_BITMAP_CANVAS_IMPL AndroidImpl
#include "graphics/BitmapCanvasAndroidImpl.h"
#else
#error "Unsupported platform"
#endif

namespace carto {

    BitmapCanvas::BitmapCanvas(int width, int height) :
        _impl(std::make_unique<CARTO_BITMAP_CANVAS_IMPL>(width, height))
    {
    }

    BitmapCanvas::~BitmapCanvas() {
    }

    void BitmapCanvas::setDrawMode(DrawMode mode) {
        _impl->setDrawMode(mode);
    }

    void BitmapCanvas::setColor(const Color& color) {
        _impl->setColor(color);
    }

    void BitmapCanvas::setStrokeWidth(float width) {
        _impl->setStrokeWidth(width);
    }

    void BitmapCanvas::setFont(const std::string& name, float size) {
        _impl->setFont(name, size);
    }

    void BitmapCanvas::pushClipRect(const ScreenBounds& clipRect) {
        _impl->pushClipRect(clipRect);
    }

    void BitmapCanvas::popClipRect() {
        _impl->popClipRect();
    }

    void BitmapCanvas::drawText(const std::string& text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        _impl->drawText(text, pos, maxWidth, breakLines);
    }

    void BitmapCanvas::drawPolygon(const std::vector<ScreenPos>& poses) {
        _impl->drawPolygon(poses);
    }

    void BitmapCanvas::drawRoundRect(const ScreenBounds& rect, float radius) {
        _impl->drawRoundRect(rect, radius);
    }

    void BitmapCanvas::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        _impl->drawBitmap(rect, bitmap);
    }

    ScreenBounds BitmapCanvas::measureTextSize(const std::string& text, int maxWidth, bool breakLines) const {
        return _impl->measureTextSize(text, maxWidth, breakLines);
    }

    std::shared_ptr<Bitmap> BitmapCanvas::buildBitmap() const {
        return _impl->buildBitmap();
    }

    BitmapCanvas::Impl::~Impl() {
    }

}
