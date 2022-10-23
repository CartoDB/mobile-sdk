#ifndef _CARTO_BITMAPCANVASEMSCRIPTENIMPL_H_
#define _CARTO_BITMAPCANVASEMSCRIPTENIMPL_H_

#include "graphics/BitmapCanvas.h"
#include <emscripten/val.h>

namespace carto {

    class BitmapCanvas::EmscriptenImpl : public BitmapCanvas::Impl {
    public:
        EmscriptenImpl(int width, int height);
        virtual ~EmscriptenImpl();

        virtual void setDrawMode(DrawMode mode);
        virtual void setColor(const Color& color);
        virtual void setStrokeWidth(float width);
        virtual void setFont(const std::string& name, float size);

        virtual void pushClipRect(const ScreenBounds& clipRect);
        virtual void popClipRect();

        virtual void drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines);
        virtual void drawPolygon(const std::vector<ScreenPos>& poses);
        virtual void drawRoundRect(const ScreenBounds& rect, float radius);
        virtual void drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap);

        virtual ScreenBounds measureTextSize(std::string text, int maxWidth, bool breakLines) const;

        virtual std::shared_ptr<Bitmap> buildBitmap() const;

    protected:
        void setCanvas(emscripten::val canvas);
        void setContext(emscripten::val context);
        void terminate();

    private:
        ScreenBounds measureTextSizeAndEllipsizeText(std::string &text, int maxWidth, bool breakLines, std::vector<std::string> &textLines, std::vector<int> &textMaxWidths, int &maxAscent, int &maxDescent) const;

        emscripten::val _canvas;
        emscripten::val _context;

        int _width;
        int _height;
        DrawMode _drawMode;
        float _strokeWidth;
        std::string _color;
        std::string _font;
        int _fontSize;
        float _lineHeight;
    };

}

#endif
