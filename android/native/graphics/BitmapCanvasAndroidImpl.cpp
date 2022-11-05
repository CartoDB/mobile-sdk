#include "graphics/BitmapCanvasAndroidImpl.h"
#include "components/Exceptions.h"
#include "utils/BitmapUtils.h"
#include "utils/AndroidUtils.h"
#include "utils/JNILocalFrame.h"
#include "utils/JNIUniqueLocalRef.h"
#include "utils/Log.h"

#include <cmath>

namespace carto {

    struct BitmapCanvas::AndroidImpl::RectFClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID constructor;

        explicit RectFClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/graphics/RectF")));
            constructor = jenv->GetMethodID(clazz, "<init>", "(FFFF)V");
        }
    };

    struct BitmapCanvas::AndroidImpl::BitmapClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID createBitmap;
        JNIUniqueGlobalRef<jobject> rgba8888BCFG;

        explicit BitmapClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/graphics/Bitmap")));
            createBitmap = jenv->GetStaticMethodID(clazz, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
            JNIUniqueLocalRef<jstring> configName(jenv, jenv->NewStringUTF("ARGB_8888"));
            JNIUniqueLocalRef<jclass> bcfgClass(jenv, jenv->FindClass("android/graphics/Bitmap$Config"));
            rgba8888BCFG = JNIUniqueGlobalRef<jobject>(jenv, jenv->NewGlobalRef(jenv->CallStaticObjectMethod(bcfgClass.get(), jenv->GetStaticMethodID(bcfgClass, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;"), configName.get())));
        }
    };

    struct BitmapCanvas::AndroidImpl::CanvasClass {
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
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/graphics/Canvas")));
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

    struct BitmapCanvas::AndroidImpl::PaintClass {
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
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/text/TextPaint")));
            constructor = jenv->GetMethodID(clazz, "<init>", "(I)V");
            setColor = jenv->GetMethodID(clazz, "setColor", "(I)V");
            setStrokeWidth = jenv->GetMethodID(clazz, "setStrokeWidth", "(F)V");
            setStyle = jenv->GetMethodID(clazz, "setStyle", "(Landroid/graphics/Paint$Style;)V");
            setTextSize = jenv->GetMethodID(clazz, "setTextSize", "(F)V");
            setTypeface = jenv->GetMethodID(clazz, "setTypeface", "(Landroid/graphics/Typeface;)Landroid/graphics/Typeface;");
            jclass styleClass = jenv->FindClass("android/graphics/Paint$Style");
            jmethodID styleValueOf = jenv->GetStaticMethodID(styleClass, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Paint$Style;");
            strokeStyle = JNIUniqueGlobalRef<jobject>(jenv, jenv->NewGlobalRef(jenv->CallStaticObjectMethod(styleClass, styleValueOf, jenv->NewStringUTF("STROKE"))));
            fillStyle = JNIUniqueGlobalRef<jobject>(jenv, jenv->NewGlobalRef(jenv->CallStaticObjectMethod(styleClass, styleValueOf, jenv->NewStringUTF("FILL"))));
        }
    };

    struct BitmapCanvas::AndroidImpl::PathClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID constructor;
        jmethodID moveTo;
        jmethodID lineTo;
        jmethodID close;

        explicit PathClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/graphics/Path")));
            constructor = jenv->GetMethodID(clazz, "<init>", "()V");
            moveTo = jenv->GetMethodID(clazz, "moveTo", "(FF)V");
            lineTo = jenv->GetMethodID(clazz, "lineTo", "(FF)V");
            close = jenv->GetMethodID(clazz, "close", "()V");
        }
    };

    struct BitmapCanvas::AndroidImpl::TypefaceClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID create;

        explicit TypefaceClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/graphics/Typeface")));
            create = jenv->GetStaticMethodID(clazz, "create", "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
        }
    };

    struct BitmapCanvas::AndroidImpl::StaticLayoutClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID constructor;
        jmethodID getLineCount;
        jmethodID getLineWidth;
        jmethodID getHeight;
        jmethodID draw;
        JNIUniqueGlobalRef<jobject> normalLayoutAlignment;

        explicit StaticLayoutClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/text/StaticLayout")));
            constructor = jenv->GetMethodID(clazz, "<init>", "(Ljava/lang/CharSequence;Landroid/text/TextPaint;ILandroid/text/Layout$Alignment;FFZ)V");
            getLineCount = jenv->GetMethodID(clazz, "getLineCount", "()I");
            getLineWidth = jenv->GetMethodID(clazz, "getLineWidth", "(I)F");
            getHeight = jenv->GetMethodID(clazz, "getHeight", "()I");
            draw = jenv->GetMethodID(clazz, "draw", "(Landroid/graphics/Canvas;)V");
            JNIUniqueLocalRef<jstring> alignmentName(jenv, jenv->NewStringUTF("ALIGN_NORMAL"));
            JNIUniqueLocalRef<jclass> layoutAlignmentClass(jenv, jenv->FindClass("android/text/Layout$Alignment"));
            normalLayoutAlignment = JNIUniqueGlobalRef<jobject>(jenv, jenv->NewGlobalRef(jenv->CallStaticObjectMethod(layoutAlignmentClass.get(), jenv->GetStaticMethodID(layoutAlignmentClass, "valueOf", "(Ljava/lang/String;)Landroid/text/Layout$Alignment;"), alignmentName.get())));
        }
    };

    struct BitmapCanvas::AndroidImpl::CharSequenceClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID toString;

        explicit CharSequenceClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("java/lang/CharSequence")));
            toString = jenv->GetMethodID(clazz, "toString", "()Ljava/lang/String;");
        }
    };

    struct BitmapCanvas::AndroidImpl::TextUtilsClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID ellipsize;

        explicit TextUtilsClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/text/TextUtils")));
            ellipsize = jenv->GetStaticMethodID(clazz, "ellipsize", "(Ljava/lang/CharSequence;Landroid/text/TextPaint;FLandroid/text/TextUtils$TruncateAt;)Ljava/lang/CharSequence;");
        }
    };

    struct BitmapCanvas::AndroidImpl::TextUtilsTruncateAtClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID valueOf;

        explicit TextUtilsTruncateAtClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("android/text/TextUtils$TruncateAt")));
            valueOf = jenv->GetStaticMethodID(clazz, "valueOf", "(Ljava/lang/String;)Landroid/text/TextUtils$TruncateAt;");
        }
    };

    BitmapCanvas::AndroidImpl::AndroidImpl(int width, int height) :
        _bitmapObject(),
        _canvasObject(),
        _paintObject()
    {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::AndroidImpl");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::AndroidImpl: JNILocalFrame not valid");
            return;
        }

        if (width > 0 && height > 0) {
            jobject bitmapObject = jenv->CallStaticObjectMethod(GetBitmapClass()->clazz, GetBitmapClass()->createBitmap, (jint)width, (jint)height, GetBitmapClass()->rgba8888BCFG.get());
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw GenericException("Failed to create Bitmap instance. Bitmap too large?");
            }
            _bitmapObject = JNIUniqueGlobalRef<jobject>(jenv, jenv->NewGlobalRef(bitmapObject));

            jobject canvasObject = jenv->NewObject(GetCanvasClass()->clazz, GetCanvasClass()->constructor, _bitmapObject.get());
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw GenericException("Failed to create Canvas instance");
            }
            _canvasObject = JNIUniqueGlobalRef<jobject>(jenv, jenv->NewGlobalRef(canvasObject));
        }

        jobject paintObject = jenv->NewObject(GetPaintClass()->clazz, GetPaintClass()->constructor, (jint)1); // 1 = ANTIALIAS_FLAG
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw GenericException("Failed to create Paint instance");
        }
        _paintObject = JNIUniqueGlobalRef<jobject>(jenv, jenv->NewGlobalRef(paintObject));
    }

    BitmapCanvas::AndroidImpl::~AndroidImpl() {
    }

    void BitmapCanvas::AndroidImpl::setDrawMode(DrawMode mode) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::setDrawMode");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::setDrawMode: JNILocalFrame not valid");
            return;
        }

        jobject styleObject = NULL;
        switch (mode) {
        case STROKE:
            styleObject = GetPaintClass()->strokeStyle;
            break;
        default:
            styleObject = GetPaintClass()->fillStyle;
            break;
        }
        jenv->CallVoidMethod(_paintObject, GetPaintClass()->setStyle, styleObject);
    }

    void BitmapCanvas::AndroidImpl::setColor(const Color& color) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::setColor");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::setColor: JNILocalFrame not valid");
            return;
        }

        jenv->CallVoidMethod(_paintObject, GetPaintClass()->setColor, (jint)color.getARGB());
    }

    void BitmapCanvas::AndroidImpl::setStrokeWidth(float width) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::setStrokeWidth");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::setStrokeWidth: JNILocalFrame not valid");
            return;
        }

        jenv->CallVoidMethod(_paintObject, GetPaintClass()->setStrokeWidth, (jfloat)width);
    }

    void BitmapCanvas::AndroidImpl::setFont(const std::string& name, float size) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::setFont");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::setFont: JNILocalFrame not valid");
            return;
        }

        jstring fontName = jenv->NewStringUTF(name.c_str());
        jobject typefaceObject = jenv->CallStaticObjectMethod(GetTypefaceClass()->clazz, GetTypefaceClass()->create, fontName, (jint)0); // 0 = NORMAL
        jenv->CallObjectMethod(_paintObject, GetPaintClass()->setTypeface, typefaceObject);
        jenv->CallVoidMethod(_paintObject, GetPaintClass()->setTextSize, (jfloat)size);
    }

    void BitmapCanvas::AndroidImpl::pushClipRect(const ScreenBounds& clipRect) {
        if (!_canvasObject) {
            return;
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::pushClipRect");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::pushClipRect: JNILocalFrame not valid");
            return;
        }

        jenv->CallIntMethod(_canvasObject, GetCanvasClass()->save);
        jenv->CallBooleanMethod(_canvasObject, GetCanvasClass()->clipRect, (jfloat)clipRect.getMin().getX(), (jfloat)clipRect.getMin().getY(), (jfloat)clipRect.getMax().getX(), (jfloat)clipRect.getMax().getY());
    }

    void BitmapCanvas::AndroidImpl::popClipRect() {
        if (!_canvasObject) {
            return;
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::popClipRect");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::popClipRect: JNILocalFrame not valid");
            return;
        }

        jenv->CallVoidMethod(_canvasObject, GetCanvasClass()->restore);
    }

    void BitmapCanvas::AndroidImpl::drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        if (!_canvasObject) {
            return;
        }

        if (text.empty()) {
            return;
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::drawText");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::drawText: JNILocalFrame not valid");
            return;
        }

        ellipsizeText(jenv, text, maxWidth, breakLines);

        jstring textObject = jenv->NewStringUTF(text.c_str());
        if (maxWidth < 0) {
            maxWidth = 65536;
        }

        jenv->CallVoidMethod(_canvasObject, GetCanvasClass()->translate, (jfloat)pos.getX(), (jfloat)pos.getY());

        jobject staticLayoutObject = jenv->NewObject(GetStaticLayoutClass()->clazz, GetStaticLayoutClass()->constructor, textObject, _paintObject.get(), (jint)maxWidth, GetStaticLayoutClass()->normalLayoutAlignment.get(), (jfloat)1, (jfloat)0, (jboolean)false);
        jenv->CallVoidMethod(staticLayoutObject, GetStaticLayoutClass()->draw, _canvasObject.get());

        jenv->CallVoidMethod(_canvasObject, GetCanvasClass()->translate, (jfloat)-pos.getX(), (jfloat)-pos.getY());
    }

    void BitmapCanvas::AndroidImpl::drawRoundRect(const ScreenBounds& rect, float radius) {
        if (!_canvasObject) {
            return;
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::drawRoundRect");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::drawRoundRect: JNILocalFrame not valid");
            return;
        }

        jobject rectFObject = jenv->NewObject(GetRectFClass()->clazz, GetRectFClass()->constructor, (jfloat)rect.getMin().getX(), (jfloat)rect.getMin().getY(), (jfloat)rect.getMax().getX(), (jfloat)rect.getMax().getY());
        jenv->CallVoidMethod(_canvasObject, GetCanvasClass()->drawRoundRect, rectFObject, (jfloat)radius, (jfloat)radius, _paintObject.get());
    }

    void BitmapCanvas::AndroidImpl::drawPolygon(const std::vector<ScreenPos>& poses) {
        if (!_canvasObject) {
            return;
        }

        if (poses.empty()) {
            return;
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::drawPolygon");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::drawPolygon: JNILocalFrame not valid");
            return;
        }

        jobject pathObject = jenv->NewObject(GetPathClass()->clazz, GetPathClass()->constructor);
        jenv->CallVoidMethod(pathObject, GetPathClass()->moveTo, (jfloat)poses[0].getX(), (jfloat)poses[0].getY());
        for (std::size_t i = 1; i < poses.size(); i++) {
            jenv->CallVoidMethod(pathObject, GetPathClass()->lineTo, (jfloat)poses[i].getX(), (jfloat)poses[i].getY());
        }
        jenv->CallVoidMethod(pathObject, GetPathClass()->close);

        jenv->CallVoidMethod(_canvasObject, GetCanvasClass()->drawPath, pathObject, _paintObject.get());
    }

    void BitmapCanvas::AndroidImpl::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        if (!_canvasObject) {
            return;
        }

        if (!bitmap) {
            return;
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::drawBitmap");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::drawBitmap: JNILocalFrame not valid");
            return;
        }

        jobject bitmapObject = BitmapUtils::CreateAndroidBitmapFromBitmap(bitmap);
        jobject rectFObject = jenv->NewObject(GetRectFClass()->clazz, GetRectFClass()->constructor, (jfloat)rect.getMin().getX(), (jfloat)rect.getMin().getY(), (jfloat)rect.getMax().getX(), (jfloat)rect.getMax().getY());
        jenv->CallVoidMethod(_canvasObject, GetCanvasClass()->drawBitmap, bitmapObject, (jobject)NULL, rectFObject, (jobject)NULL);
    }

    ScreenBounds BitmapCanvas::AndroidImpl::measureTextSize(std::string text, int maxWidth, bool breakLines) const {
        if (text.empty()) {
            return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
        }

        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::measureTextSize");
        if (!jframe.isValid()) {
            Log::Error("BitmapCanvas::AndroidImpl::measureTextSize: JNILocalFrame not valid");
            return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
        }

        ellipsizeText(jenv, text, maxWidth, breakLines);

        jstring textObject = jenv->NewStringUTF(text.c_str());
        if (maxWidth < 0) {
            maxWidth = 65536;
        }
        jobject staticLayoutObject = jenv->NewObject(GetStaticLayoutClass()->clazz, GetStaticLayoutClass()->constructor, textObject, _paintObject.get(), (jint)maxWidth, GetStaticLayoutClass()->normalLayoutAlignment.get(), (jfloat)1, (jfloat)0, (jboolean)false);

        jfloat width = 0;
        for (jint line = jenv->CallIntMethod(staticLayoutObject, GetStaticLayoutClass()->getLineCount); --line >= 0; ) {
            width = std::max(width, jenv->CallFloatMethod(staticLayoutObject, GetStaticLayoutClass()->getLineWidth, (jint)line));
        }
        jfloat height = (jfloat)jenv->CallIntMethod(staticLayoutObject, GetStaticLayoutClass()->getHeight);

        return ScreenBounds(ScreenPos(0, 0), ScreenPos(std::ceil(width), std::ceil(height)));
    }

    std::shared_ptr<Bitmap> BitmapCanvas::AndroidImpl::buildBitmap() const {
        if (!_bitmapObject) {
            return std::shared_ptr<Bitmap>();
        }

        std::shared_ptr<Bitmap> bitmap = BitmapUtils::CreateBitmapFromAndroidBitmap(_bitmapObject);
        if (!bitmap) {
            const unsigned char pixel[] = { 0, 0, 0, 0 };
            bitmap = std::make_shared<Bitmap>(pixel, 1, 1, ColorFormat::COLOR_FORMAT_RGBA, 4);
        }
        return bitmap;
    }

    void BitmapCanvas::AndroidImpl::ellipsizeText(JNIEnv* jenv, std::string& text, int maxWidth, bool breakLines) const {
        if (maxWidth < 0 || breakLines) {
            return;
        }

        jstring truncateAtName = jenv->NewStringUTF("END");
        jobject truncateAtObject = jenv->CallStaticObjectMethod(GetTextUtilsTruncateAtClass()->clazz, GetTextUtilsTruncateAtClass()->valueOf, truncateAtName);

        jstring textObject = jenv->NewStringUTF(text.c_str());
        jobject charSeqObject = jenv->CallStaticObjectMethod(GetTextUtilsClass()->clazz, GetTextUtilsClass()->ellipsize, textObject, _paintObject.get(), (jfloat)maxWidth, truncateAtObject);
        jstring ellipsizedTextObject = (jstring)jenv->CallObjectMethod(charSeqObject, GetCharSequenceClass()->toString);

        const char* ellipsizedTextStr = jenv->GetStringUTFChars(ellipsizedTextObject, NULL);
        text = ellipsizedTextStr;
        jenv->ReleaseStringUTFChars(ellipsizedTextObject, ellipsizedTextStr);
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::RectFClass>& BitmapCanvas::AndroidImpl::GetRectFClass() {
        static std::unique_ptr<RectFClass> cls(new RectFClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::BitmapClass>& BitmapCanvas::AndroidImpl::GetBitmapClass() {
        static std::unique_ptr<BitmapClass> cls(new BitmapClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::CanvasClass>& BitmapCanvas::AndroidImpl::GetCanvasClass() {
        static std::unique_ptr<CanvasClass> cls(new CanvasClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::PaintClass>& BitmapCanvas::AndroidImpl::GetPaintClass() {
        static std::unique_ptr<PaintClass> cls(new PaintClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::PathClass>& BitmapCanvas::AndroidImpl::GetPathClass() {
        static std::unique_ptr<PathClass> cls(new PathClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::TypefaceClass>& BitmapCanvas::AndroidImpl::GetTypefaceClass() {
        static std::unique_ptr<TypefaceClass> cls(new TypefaceClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::StaticLayoutClass>& BitmapCanvas::AndroidImpl::GetStaticLayoutClass() {
        static std::unique_ptr<StaticLayoutClass> cls(new StaticLayoutClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::CharSequenceClass>& BitmapCanvas::AndroidImpl::GetCharSequenceClass() {
        static std::unique_ptr<CharSequenceClass> cls(new CharSequenceClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::TextUtilsClass>& BitmapCanvas::AndroidImpl::GetTextUtilsClass() {
        static std::unique_ptr<TextUtilsClass> cls(new TextUtilsClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::TextUtilsTruncateAtClass>& BitmapCanvas::AndroidImpl::GetTextUtilsTruncateAtClass() {
        static std::unique_ptr<TextUtilsTruncateAtClass> cls(new TextUtilsTruncateAtClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

}
