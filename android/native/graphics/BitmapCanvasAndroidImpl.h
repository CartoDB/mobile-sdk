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
        struct RectFClass {
            JNIUniqueGlobalRef<jclass> clazz;
            jmethodID constructor;

            explicit RectFClass(JNIEnv* jenv) {
                clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("android/graphics/RectF")));
                constructor = jenv->GetMethodID(clazz, "<init>", "(FFFF)V");
            }
        };

        struct BitmapClass {
            JNIUniqueGlobalRef<jclass> clazz;
            jmethodID createBitmap;
            JNIUniqueGlobalRef<jobject> rgba8888BCFG;

            explicit BitmapClass(JNIEnv* jenv) {
                clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("android/graphics/Bitmap")));
                createBitmap = jenv->GetStaticMethodID(clazz, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
                jstring configName = jenv->NewStringUTF("ARGB_8888");
                jclass bcfgClass = jenv->FindClass("android/graphics/Bitmap$Config");
                rgba8888BCFG = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(jenv->CallStaticObjectMethod(bcfgClass, jenv->GetStaticMethodID(bcfgClass, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;"), configName)));
            }
        };

        struct CanvasClass {
            JNIUniqueGlobalRef<jclass> clazz;
            jmethodID constructor;
            jmethodID restore;
            jmethodID save;
            jmethodID clipRect;
            jmethodID translate;
            jmethodID drawRoundRect;
            jmethodID drawBitmap;
            jmethodID drawPath;

            explicit CanvasClass(JNIEnv* jenv) {
                clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("android/graphics/Canvas")));
                constructor = jenv->GetMethodID(clazz, "<init>", "(Landroid/graphics/Bitmap;)V");
                restore = jenv->GetMethodID(clazz, "restore", "()V");
                save = jenv->GetMethodID(clazz, "save", "()I");
                clipRect = jenv->GetMethodID(clazz, "clipRect", "(FFFF)Z");
                translate = jenv->GetMethodID(clazz, "translate", "(FF)V");
                drawRoundRect = jenv->GetMethodID(clazz, "drawRoundRect", "(Landroid/graphics/RectF;FFLandroid/graphics/Paint;)V");
                drawBitmap = jenv->GetMethodID(clazz, "drawBitmap", "(Landroid/graphics/Bitmap;Landroid/graphics/Rect;Landroid/graphics/RectF;Landroid/graphics/Paint;)V");
                drawPath = jenv->GetMethodID(clazz, "drawPath", "(Landroid/graphics/Path;Landroid/graphics/Paint;)V");
            }
        };

        struct PaintClass {
            JNIUniqueGlobalRef<jclass> clazz;
            jmethodID constructor;
            jmethodID setColor;
            jmethodID setStrokeWidth;
            jmethodID setStyle;
            jmethodID setTextSize;
            jmethodID setTypeface;
            JNIUniqueGlobalRef<jobject> strokeStyle;
            JNIUniqueGlobalRef<jobject> fillStyle;

            explicit PaintClass(JNIEnv* jenv) {
                clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("android/text/TextPaint")));
                constructor = jenv->GetMethodID(clazz, "<init>", "(I)V");
                setColor = jenv->GetMethodID(clazz, "setColor", "(I)V");
                setStrokeWidth = jenv->GetMethodID(clazz, "setStrokeWidth", "(F)V");
                setStyle = jenv->GetMethodID(clazz, "setStyle", "(Landroid/graphics/Paint$Style;)V");
                setTextSize = jenv->GetMethodID(clazz, "setTextSize", "(F)V");
                setTypeface = jenv->GetMethodID(clazz, "setTypeface", "(Landroid/graphics/Typeface;)Landroid/graphics/Typeface;");
                jclass styleClass = jenv->FindClass("android/graphics/Paint$Style");
                jmethodID styleValueOf = jenv->GetStaticMethodID(styleClass, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Paint$Style;");
                strokeStyle = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(jenv->CallStaticObjectMethod(styleClass, styleValueOf, jenv->NewStringUTF("STROKE"))));
                fillStyle = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(jenv->CallStaticObjectMethod(styleClass, styleValueOf, jenv->NewStringUTF("FILL"))));
            }
        };

        struct TypefaceClass {
            JNIUniqueGlobalRef<jclass> clazz;
            jmethodID create;

            explicit TypefaceClass(JNIEnv* jenv) {
                clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("android/graphics/Typeface")));
                create = jenv->GetStaticMethodID(clazz, "create", "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
            }
        };

        struct StaticLayoutClass {
            JNIUniqueGlobalRef<jclass> clazz;
            jmethodID constructor;
            jmethodID getLineCount;
            jmethodID getLineWidth;
            jmethodID getHeight;
            jmethodID draw;
            JNIUniqueGlobalRef<jobject> normalLayoutAlignment;

            explicit StaticLayoutClass(JNIEnv* jenv) {
                clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("android/text/StaticLayout")));
                constructor = jenv->GetMethodID(clazz, "<init>", "(Ljava/lang/CharSequence;Landroid/text/TextPaint;ILandroid/text/Layout$Alignment;FFZ)V");
                getLineCount = jenv->GetMethodID(clazz, "getLineCount", "()I");
                getLineWidth = jenv->GetMethodID(clazz, "getLineWidth", "(I)F");
                getHeight = jenv->GetMethodID(clazz, "getHeight", "()I");
                draw = jenv->GetMethodID(clazz, "draw", "(Landroid/graphics/Canvas;)V");
                jstring alignmentName = jenv->NewStringUTF("ALIGN_NORMAL");
                jclass layoutAlignmentClass = jenv->FindClass("android/text/Layout$Alignment");
                normalLayoutAlignment = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(jenv->CallStaticObjectMethod(layoutAlignmentClass, jenv->GetStaticMethodID(layoutAlignmentClass, "valueOf", "(Ljava/lang/String;)Landroid/text/Layout$Alignment;"), alignmentName)));
            }
        };

        void ellipsizeText(JNIEnv* jenv, std::string& text, int maxWidth, bool breakLines) const;

        static std::unique_ptr<RectFClass>& GetRectFClass();
        static std::unique_ptr<BitmapClass>& GetBitmapClass();
        static std::unique_ptr<CanvasClass>& GetCanvasClass();
        static std::unique_ptr<PaintClass>& GetPaintClass();
        static std::unique_ptr<TypefaceClass>& GetTypefaceClass();
        static std::unique_ptr<StaticLayoutClass>& GetStaticLayoutClass();

        JNIUniqueGlobalRef<jobject> _bitmapObject;
        JNIUniqueGlobalRef<jobject> _canvasObject;
        JNIUniqueGlobalRef<jobject> _paintObject;

    };
}

#endif
