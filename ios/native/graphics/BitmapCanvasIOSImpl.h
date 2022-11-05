/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPCANVASIOSIMPL_H_
#define _CARTO_BITMAPCANVASIOSIMPL_H_

#include "graphics/BitmapCanvas.h"
#include "utils/CFUniquePtr.h"

#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>

namespace carto {

    class BitmapCanvas::IOSImpl : public BitmapCanvas::Impl {
    public:
        IOSImpl(int width, int height);
        virtual ~IOSImpl();

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

    private:
        CFAttributedStringRef createCFAttributedString(const std::string& text) const;
        CGSize measureFramesetter(CTFramesetterRef framesetter, int maxWidth, bool breakLines) const;

        int _width;
        int _height;
        std::vector<unsigned char> _data;
        CFUniquePtr<CGColorSpaceRef> _colorSpace;
        CFUniquePtr<CGContextRef> _context;
        DrawMode _drawMode;
        float _strokeWidth;
        mutable CFUniquePtr<CFNumberRef> _strokeWidthRef; // temporary object
        CFUniquePtr<CGColorRef> _color;
        CFUniquePtr<CTFontRef> _font;
        float _fontSize;
    };

}

#endif
