#include "graphics/BitmapCanvas.h"
#include "graphics/Bitmap.h"
#include "utils/BitmapUtils.h"
#include "utils/AndroidUtils.h"
#include "utils/JNIUniqueGlobalRef.h"
#include "utils/Log.h"

#include <cmath>

#include <jni.h>

namespace {
    using carto::JNIUniqueGlobalRef;

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

    void ellipsizeText(JNIEnv* jenv, jobject paintObject, std::string& text, int maxWidth, bool breakLines) {
        if (maxWidth < 0 || breakLines) {
            return;
        }

        jstring truncateAtName = jenv->NewStringUTF("END");
        jclass truncateAtClass = jenv->FindClass("android/text/TextUtils$TruncateAt");
        jobject truncateAtObject = jenv->CallStaticObjectMethod(truncateAtClass, jenv->GetStaticMethodID(truncateAtClass, "valueOf", "(Ljava/lang/String;)Landroid/text/TextUtils$TruncateAt;"), truncateAtName);

        jstring textObject = jenv->NewStringUTF(text.c_str());

        jclass textUtilsClass = jenv->FindClass("android/text/TextUtils");
        jmethodID ellipsize = jenv->GetStaticMethodID(textUtilsClass, "ellipsize", "(Ljava/lang/CharSequence;Landroid/text/TextPaint;FLandroid/text/TextUtils$TruncateAt;)Ljava/lang/CharSequence;");
        jobject charSeqObject = jenv->CallStaticObjectMethod(textUtilsClass, ellipsize, textObject, paintObject, (jfloat)maxWidth, truncateAtObject);

        jclass charSeqClass = jenv->FindClass("java/lang/CharSequence");
        jmethodID toString = jenv->GetMethodID(charSeqClass, "toString", "()Ljava/lang/String;");
        textObject = (jstring)jenv->CallObjectMethod(charSeqObject, toString);

        const char* textStr = jenv->GetStringUTFChars(textObject, NULL);
        text = textStr;
        jenv->ReleaseStringUTFChars(textObject, textStr);
    }

}

namespace carto {

    struct BitmapCanvas::State {
        JNIUniqueGlobalRef<jobject> _bitmapObject;
        JNIUniqueGlobalRef<jobject> _canvasObject;
        JNIUniqueGlobalRef<jobject> _paintObject;

        static std::unique_ptr<RectFClass> _RectFClass;
        static std::unique_ptr<BitmapClass> _BitmapClass;
        static std::unique_ptr<CanvasClass> _CanvasClass;
        static std::unique_ptr<PaintClass> _PaintClass;
        static std::unique_ptr<TypefaceClass> _TypefaceClass;
        static std::unique_ptr<StaticLayoutClass> _StaticLayoutClass;
        static std::mutex _Mutex;

        State() { }
    };

    std::unique_ptr<RectFClass> BitmapCanvas::State::_RectFClass;
    std::unique_ptr<BitmapClass> BitmapCanvas::State::_BitmapClass;
    std::unique_ptr<CanvasClass> BitmapCanvas::State::_CanvasClass;
    std::unique_ptr<PaintClass> BitmapCanvas::State::_PaintClass;
    std::unique_ptr<TypefaceClass> BitmapCanvas::State::_TypefaceClass;
    std::unique_ptr<StaticLayoutClass> BitmapCanvas::State::_StaticLayoutClass;
    std::mutex BitmapCanvas::State::_Mutex;

    BitmapCanvas::BitmapCanvas(int width, int height) :
        _state(new State())
    {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::BitmapCanvas");
        if (!jframe.isValid()) {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(State::_Mutex);
            if (!State::_RectFClass) {
                State::_RectFClass = std::unique_ptr<RectFClass>(new RectFClass(jenv));
            }
            if (!State::_BitmapClass) {
                State::_BitmapClass = std::unique_ptr<BitmapClass>(new BitmapClass(jenv));
            }
            if (!State::_CanvasClass) {
                State::_CanvasClass = std::unique_ptr<CanvasClass>(new CanvasClass(jenv));
            }
            if (!State::_PaintClass) {
                State::_PaintClass = std::unique_ptr<PaintClass>(new PaintClass(jenv));
            }
            if (!State::_TypefaceClass) {
                State::_TypefaceClass = std::unique_ptr<TypefaceClass>(new TypefaceClass(jenv));
            }
            if (!State::_StaticLayoutClass) {
                State::_StaticLayoutClass = std::unique_ptr<StaticLayoutClass>(new StaticLayoutClass(jenv));
            }
        }

        if (width > 0 && height > 0) {
            _state->_bitmapObject = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(jenv->CallStaticObjectMethod(_state->_BitmapClass->clazz, _state->_BitmapClass->createBitmap, (jint)width, (jint)height, _state->_BitmapClass->rgba8888BCFG.get())));
            _state->_canvasObject = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(jenv->NewObject(_state->_CanvasClass->clazz, _state->_CanvasClass->constructor, _state->_bitmapObject.get())));
        }
        _state->_paintObject = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(jenv->NewObject(_state->_PaintClass->clazz, _state->_PaintClass->constructor, (jint)1))); // 1 = ANTIALIAS_FLAG
    }

    BitmapCanvas::~BitmapCanvas() {
    }

    void BitmapCanvas::setDrawMode(DrawMode mode) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::setDrawMode");
        if (!jframe.isValid()) {
            return;
        }

        jobject styleObject = NULL;
        switch (mode) {
        case STROKE:
            styleObject = _state->_PaintClass->strokeStyle;
            break;
        default:
            styleObject = _state->_PaintClass->fillStyle;
            break;
        }
        jenv->CallVoidMethod(_state->_paintObject, _state->_PaintClass->setStyle, styleObject);
    }

    void BitmapCanvas::setColor(const Color& color) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::setColor");
        if (!jframe.isValid()) {
            return;
        }

        jenv->CallVoidMethod(_state->_paintObject, _state->_PaintClass->setColor, (jint)color.getARGB());
    }

    void BitmapCanvas::setStrokeWidth(float width) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::setStrokeWidth");
        if (!jframe.isValid()) {
            return;
        }

        jenv->CallVoidMethod(_state->_paintObject, _state->_PaintClass->setStrokeWidth, (jfloat)width);
    }

    void BitmapCanvas::setFont(const std::string& name, float size) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::setFont");
        if (!jframe.isValid()) {
            return;
        }

        jstring fontName = jenv->NewStringUTF(name.c_str());
        jobject typefaceObject = jenv->CallStaticObjectMethod(_state->_TypefaceClass->clazz, _state->_TypefaceClass->create, fontName, (jint)0); // 0 = NORMAL
        jenv->CallObjectMethod(_state->_paintObject, _state->_PaintClass->setTypeface, typefaceObject);
        jenv->CallVoidMethod(_state->_paintObject, _state->_PaintClass->setTextSize, (jfloat)size);
    }

    void BitmapCanvas::pushClipRect(const ScreenBounds& clipRect) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::pushClipRect");
        if (!jframe.isValid()) {
            return;
        }

        jenv->CallIntMethod(_state->_canvasObject, _state->_CanvasClass->save);
        jenv->CallBooleanMethod(_state->_canvasObject, _state->_CanvasClass->clipRect, (jfloat)clipRect.getMin().getX(), (jfloat)clipRect.getMin().getY(), (jfloat)clipRect.getMax().getX(), (jfloat)clipRect.getMax().getY());
    }

    void BitmapCanvas::popClipRect() {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::popClipRect");
        if (!jframe.isValid()) {
            return;
        }

        jenv->CallVoidMethod(_state->_canvasObject, _state->_CanvasClass->restore);
    }

    void BitmapCanvas::drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::drawText");
        if (!jframe.isValid()) {
            return;
        }

        ellipsizeText(jenv, _state->_paintObject, text, maxWidth, breakLines);

        jstring textObject = jenv->NewStringUTF(text.c_str());
        if (maxWidth < 0) {
            maxWidth = 65536;
        }

        jenv->CallVoidMethod(_state->_canvasObject, _state->_CanvasClass->translate, (jfloat)pos.getX(), (jfloat)pos.getY());

        jobject staticLayoutObject = jenv->NewObject(_state->_StaticLayoutClass->clazz, _state->_StaticLayoutClass->constructor, textObject, _state->_paintObject.get(), (jint)maxWidth, _state->_StaticLayoutClass->normalLayoutAlignment.get(), (jfloat)1, (jfloat)0, (jboolean)false);
        jenv->CallVoidMethod(staticLayoutObject, _state->_StaticLayoutClass->draw, _state->_canvasObject.get());

        jenv->CallVoidMethod(_state->_canvasObject, _state->_CanvasClass->translate, (jfloat)-pos.getX(), (jfloat)-pos.getY());
    }

    void BitmapCanvas::drawRoundRect(const ScreenBounds& rect, float radius) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::drawRoundRect");
        if (!jframe.isValid()) {
            return;
        }

        jobject rectFObject = jenv->NewObject(_state->_RectFClass->clazz, _state->_RectFClass->constructor, (jfloat)rect.getMin().getX(), (jfloat)rect.getMin().getY(), (jfloat)rect.getMax().getX(), (jfloat)rect.getMax().getY());
        jenv->CallVoidMethod(_state->_canvasObject, _state->_CanvasClass->drawRoundRect, rectFObject, (jfloat)radius, (jfloat)radius, _state->_paintObject.get());
    }

    void BitmapCanvas::drawPolygon(const std::vector<ScreenPos>& poses) {
        if (poses.empty()) {
            return;
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::drawPolygon");
        if (!jframe.isValid()) {
            return;
        }

        jclass pathClass = jenv->FindClass("android/graphics/Path");
        jmethodID pathConstructor = jenv->GetMethodID(pathClass, "<init>", "()V");
        jobject pathObject = jenv->NewObject(pathClass, pathConstructor);

        jmethodID moveTo = jenv->GetMethodID(pathClass, "moveTo", "(FF)V");
        jenv->CallVoidMethod(pathObject, moveTo, (jfloat)poses[0].getX(), (jfloat)poses[0].getY());

        jmethodID lineTo = jenv->GetMethodID(pathClass, "lineTo", "(FF)V");
        for (size_t i = 1; i < poses.size(); i++) {
            jenv->CallVoidMethod(pathObject, lineTo, (jfloat)poses[i].getX(), (jfloat)poses[i].getY());
        }

        jmethodID close = jenv->GetMethodID(pathClass, "close", "()V");
        jenv->CallVoidMethod(pathObject, close);

        jenv->CallVoidMethod(_state->_canvasObject, _state->_CanvasClass->drawPath, pathObject, _state->_paintObject.get());
    }

    void BitmapCanvas::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            return;
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::drawBitmap");
        if (!jframe.isValid()) {
            return;
        }

        jobject bitmapObject = BitmapUtils::CreateAndroidBitmapFromBitmap(bitmap);
        jobject rectFObject = jenv->NewObject(_state->_RectFClass->clazz, _state->_RectFClass->constructor, (jfloat)rect.getMin().getX(), (jfloat)rect.getMin().getY(), (jfloat)rect.getMax().getX(), (jfloat)rect.getMax().getY());
        jenv->CallVoidMethod(_state->_canvasObject, _state->_CanvasClass->drawBitmap, bitmapObject, (jobject)NULL, rectFObject, (jobject)NULL);
    }

    ScreenBounds BitmapCanvas::measureTextSize(std::string text, int maxWidth, bool breakLines) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::measureTextSize");
        if (!jframe.isValid()) {
            return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
        }

        ellipsizeText(jenv, _state->_paintObject, text, maxWidth, breakLines);

        jstring textObject = jenv->NewStringUTF(text.c_str());
        if (maxWidth < 0) {
            maxWidth = 65536;
        }
        jobject staticLayoutObject = jenv->NewObject(_state->_StaticLayoutClass->clazz, _state->_StaticLayoutClass->constructor, textObject, _state->_paintObject.get(), (jint)maxWidth, _state->_StaticLayoutClass->normalLayoutAlignment.get(), (jfloat)1, (jfloat)0, (jboolean)false);

        jfloat width = 0;
        for (jint line = jenv->CallIntMethod(staticLayoutObject, _state->_StaticLayoutClass->getLineCount); --line >= 0; ) {
            width = std::max(width, jenv->CallFloatMethod(staticLayoutObject, _state->_StaticLayoutClass->getLineWidth, (jint)line));
        }
        jfloat height = (jfloat)jenv->CallIntMethod(staticLayoutObject, _state->_StaticLayoutClass->getHeight);

        return ScreenBounds(ScreenPos(0, 0), ScreenPos(std::ceil(width), std::ceil(height)));
    }

    std::shared_ptr<Bitmap> BitmapCanvas::buildBitmap() const {
        std::shared_ptr<Bitmap> bitmap = BitmapUtils::CreateBitmapFromAndroidBitmap(_state->_bitmapObject);
        if (!bitmap) {
            const unsigned char pixel[] = { 0, 0, 0, 0 };
            bitmap = std::make_shared<Bitmap>(pixel, 1, 1, ColorFormat::COLOR_FORMAT_RGBA, 4);
        }
        return bitmap;
    }

}
