/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPCANVAS_H_
#define _CARTO_BITMAPCANVAS_H_

#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "graphics/Color.h"
#include "graphics/Bitmap.h"

#include <memory>
#include <string>
#include <vector>

namespace carto {

    class BitmapCanvas {
    public:
        enum DrawMode {
            FILL,
            STROKE
        };

        BitmapCanvas(int width, int height);
        virtual ~BitmapCanvas();

        void setDrawMode(DrawMode mode);
        void setColor(const Color& color);
        void setStrokeWidth(float width);
        void setFont(const std::string& name, float size);

        void pushClipRect(const ScreenBounds& clipRect);
        void popClipRect();

        void drawText(const std::string& text, const ScreenPos& pos, int maxWidth, bool breakLines);
        void drawPolygon(const std::vector<ScreenPos>& poses);
        void drawRoundRect(const ScreenBounds& rect, float radius);
        void drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap);

        ScreenBounds measureTextSize(const std::string& text, int maxWidth, bool breakLines) const;

        std::shared_ptr<Bitmap> buildBitmap() const;

    protected:
        class Impl {
        public:
            virtual ~Impl();

            virtual void setDrawMode(DrawMode mode) = 0;
            virtual void setColor(const Color& color) = 0;
            virtual void setStrokeWidth(float width) = 0;
            virtual void setFont(const std::string& name, float size) = 0;

            virtual void pushClipRect(const ScreenBounds& clipRect) = 0;
            virtual void popClipRect() = 0;

            virtual void drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) = 0;
            virtual void drawPolygon(const std::vector<ScreenPos>& poses) = 0;
            virtual void drawRoundRect(const ScreenBounds& rect, float radius) = 0;
            virtual void drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) = 0;

            virtual ScreenBounds measureTextSize(std::string text, int maxWidth, bool breakLines) const = 0;

            virtual std::shared_ptr<Bitmap> buildBitmap() const = 0;
        };

        class AndroidImpl;
        class IOSImpl;
        class UWPImpl;
        
        std::unique_ptr<Impl> _impl;
    };

}

#endif
