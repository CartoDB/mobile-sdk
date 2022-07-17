#include "graphics/BitmapCanvasEmscriptenImpl.h"
#include "components/Exceptions.h"
#include "utils/BitmapUtils.h"

#include <cmath>

namespace carto {
    BitmapCanvas::EmscriptenImpl::EmscriptenImpl(int width, int height)
    {
        
    }

    BitmapCanvas::EmscriptenImpl::~EmscriptenImpl() {
    }

    void BitmapCanvas::EmscriptenImpl::setDrawMode(DrawMode mode) {
        
    }

    void BitmapCanvas::EmscriptenImpl::setColor(const Color& color) {
        
    }

    void BitmapCanvas::EmscriptenImpl::setStrokeWidth(float width) {
        
    }

    void BitmapCanvas::EmscriptenImpl::setFont(const std::string& name, float size) {
        
    }

    void BitmapCanvas::EmscriptenImpl::pushClipRect(const ScreenBounds& clipRect) {
       
    }

    void BitmapCanvas::EmscriptenImpl::popClipRect() {
        
    }

    void BitmapCanvas::EmscriptenImpl::drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        
    }

    void BitmapCanvas::EmscriptenImpl::drawRoundRect(const ScreenBounds& rect, float radius) {
       
    }

    void BitmapCanvas::EmscriptenImpl::drawPolygon(const std::vector<ScreenPos>& poses) {
       
    }

    void BitmapCanvas::EmscriptenImpl::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        
    }

    ScreenBounds BitmapCanvas::EmscriptenImpl::measureTextSize(std::string text, int maxWidth, bool breakLines) const {
        return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
    }

    std::shared_ptr<Bitmap> BitmapCanvas::EmscriptenImpl::buildBitmap() const {
        return std::shared_ptr<Bitmap>();
    }
}
