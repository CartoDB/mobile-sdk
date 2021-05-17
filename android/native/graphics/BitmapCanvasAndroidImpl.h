/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPCANVASANDROIDIMPL_H_
#define _CARTO_BITMAPCANVASANDROIDIMPL_H_

#include "graphics/BitmapCanvas.h"
#include "utils/JNIUniqueGlobalRef.h"

namespace carto {

    class BitmapCanvas::AndroidImpl : public BitmapCanvas::Impl {
    public:
        AndroidImpl(int width, int height);
        virtual ~AndroidImpl();

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
        struct RectFClass;
        struct BitmapClass;
        struct CanvasClass;
        struct PaintClass;
        struct PathClass;
        struct TypefaceClass;
        struct StaticLayoutClass;
        struct CharSequenceClass;
        struct TextUtilsClass;
        struct TextUtilsTruncateAtClass;

        void ellipsizeText(JNIEnv* jenv, std::string& text, int maxWidth, bool breakLines) const;

        static std::unique_ptr<RectFClass>& GetRectFClass();
        static std::unique_ptr<BitmapClass>& GetBitmapClass();
        static std::unique_ptr<CanvasClass>& GetCanvasClass();
        static std::unique_ptr<PaintClass>& GetPaintClass();
        static std::unique_ptr<PathClass>& GetPathClass();
        static std::unique_ptr<TypefaceClass>& GetTypefaceClass();
        static std::unique_ptr<StaticLayoutClass>& GetStaticLayoutClass();
        static std::unique_ptr<CharSequenceClass>& GetCharSequenceClass();
        static std::unique_ptr<TextUtilsClass>& GetTextUtilsClass();
        static std::unique_ptr<TextUtilsTruncateAtClass>& GetTextUtilsTruncateAtClass();

        JNIUniqueGlobalRef<jobject> _bitmapObject;
        JNIUniqueGlobalRef<jobject> _canvasObject;
        JNIUniqueGlobalRef<jobject> _paintObject;

    };

}

#endif
