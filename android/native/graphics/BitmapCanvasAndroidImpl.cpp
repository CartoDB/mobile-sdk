#include "graphics/BitmapCanvasAndroidImpl.h"
#include "graphics/Bitmap.h"
#include "components/Exceptions.h"
#include "utils/BitmapUtils.h"
#include "utils/AndroidUtils.h"
#include "utils/Log.h"

#include <cmath>

namespace carto {

    BitmapCanvas::AndroidImpl::AndroidImpl(int width, int height) :
        _bitmapObject(),
        _canvasObject(),
        _paintObject()
    {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::BitmapCanvas::AndroidImpl");
        if (!jframe.isValid()) {
            return;
        }

        if (width > 0 && height > 0) {
            jobject bitmapObject = jenv->CallStaticObjectMethod(GetBitmapClass()->clazz, GetBitmapClass()->createBitmap, (jint)width, (jint)height, GetBitmapClass()->rgba8888BCFG.get());
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw GenericException("Failed to create Bitmap instance. Bitmap too large?");
            }
            _bitmapObject = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(bitmapObject));

            jobject canvasObject = jenv->NewObject(GetCanvasClass()->clazz, GetCanvasClass()->constructor, _bitmapObject.get());
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw GenericException("Failed to create Canvas instance");
            }
            _canvasObject = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(canvasObject));
        }

        jobject paintObject = jenv->NewObject(GetPaintClass()->clazz, GetPaintClass()->constructor, (jint)1); // 1 = ANTIALIAS_FLAG
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw GenericException("Failed to create Paint instance");
        }
        _paintObject = JNIUniqueGlobalRef<jobject>(jenv->NewGlobalRef(paintObject));
    }

    BitmapCanvas::AndroidImpl::~AndroidImpl() {
    }

    void BitmapCanvas::AndroidImpl::setDrawMode(DrawMode mode) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::setDrawMode");
        if (!jframe.isValid()) {
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
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::setColor");
        if (!jframe.isValid()) {
            return;
        }

        jenv->CallVoidMethod(_paintObject, GetPaintClass()->setColor, (jint)color.getARGB());
    }

    void BitmapCanvas::AndroidImpl::setStrokeWidth(float width) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::setStrokeWidth");
        if (!jframe.isValid()) {
            return;
        }

        jenv->CallVoidMethod(_paintObject, GetPaintClass()->setStrokeWidth, (jfloat)width);
    }

    void BitmapCanvas::AndroidImpl::setFont(const std::string& name, float size) {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::setFont");
        if (!jframe.isValid()) {
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
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::pushClipRect");
        if (!jframe.isValid()) {
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
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::popClipRect");
        if (!jframe.isValid()) {
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
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::drawText");
        if (!jframe.isValid()) {
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
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::drawRoundRect");
        if (!jframe.isValid()) {
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
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::drawPolygon");
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
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::drawBitmap");
        if (!jframe.isValid()) {
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
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "BitmapCanvas::AndroidImpl::measureTextSize");
        if (!jframe.isValid()) {
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
        jclass truncateAtClass = jenv->FindClass("android/text/TextUtils$TruncateAt");
        jobject truncateAtObject = jenv->CallStaticObjectMethod(truncateAtClass, jenv->GetStaticMethodID(truncateAtClass, "valueOf", "(Ljava/lang/String;)Landroid/text/TextUtils$TruncateAt;"), truncateAtName);

        jstring textObject = jenv->NewStringUTF(text.c_str());

        jclass textUtilsClass = jenv->FindClass("android/text/TextUtils");
        jmethodID ellipsize = jenv->GetStaticMethodID(textUtilsClass, "ellipsize", "(Ljava/lang/CharSequence;Landroid/text/TextPaint;FLandroid/text/TextUtils$TruncateAt;)Ljava/lang/CharSequence;");
        jobject charSeqObject = jenv->CallStaticObjectMethod(textUtilsClass, ellipsize, textObject, _paintObject.get(), (jfloat)maxWidth, truncateAtObject);

        jclass charSeqClass = jenv->FindClass("java/lang/CharSequence");
        jmethodID toString = jenv->GetMethodID(charSeqClass, "toString", "()Ljava/lang/String;");
        textObject = (jstring)jenv->CallObjectMethod(charSeqObject, toString);

        const char* textStr = jenv->GetStringUTFChars(textObject, NULL);
        text = textStr;
        jenv->ReleaseStringUTFChars(textObject, textStr);
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

    std::unique_ptr<BitmapCanvas::AndroidImpl::TypefaceClass>& BitmapCanvas::AndroidImpl::GetTypefaceClass() {
        static std::unique_ptr<TypefaceClass> cls(new TypefaceClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<BitmapCanvas::AndroidImpl::StaticLayoutClass>& BitmapCanvas::AndroidImpl::GetStaticLayoutClass() {
        static std::unique_ptr<StaticLayoutClass> cls(new StaticLayoutClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

}
