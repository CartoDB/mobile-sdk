#include <emscripten/bind.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <emscripten/proxying.h>

#include <cmath>
#include <sstream>
#include <algorithm>

#include "ui/MapView.h"

#define protected friend class EmRunOnMainThread; protected

#include "graphics/BitmapCanvasEmscriptenImpl.h"
#include "components/Exceptions.h"
#include "utils/BitmapUtils.h"
#include "utils/Log.h"


typedef union em_variant_val {
    int i;
    int64_t i64;
    float f;
    double d;
    void *vp;
    char *cp;
} em_variant_val;

#define EM_QUEUED_CALL_MAX_ARGS 11
typedef struct em_queued_call {
    int functionEnum;
    void *functionPtr;
    _Atomic uint32_t operationDone;
    em_variant_val args[EM_QUEUED_JS_CALL_MAX_ARGS];
    em_variant_val returnValue;
    void *satelliteData;
    int calleeDelete;
} em_queued_call;

namespace carto {
    class EmRunOnMainThread {
    public:
        static void destructor(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            self->terminate();
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void constructor(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            const emscripten::val document = emscripten::val::global("document");
            emscripten::val canvas = document.call<emscripten::val>("createElement", std::string("canvas"));
            canvas.set("width", q->args[1].i);
            canvas.set("height", q->args[2].i);
            self->setCanvas(canvas);
            self->setContext(canvas.call<emscripten::val>("getContext", std::string("2d")));
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void pushClipRect(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            const carto::ScreenBounds clipRect = *((const carto::ScreenBounds*) q->args[1].vp);
            self->pushClipRect(clipRect);
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void popClipRect(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            self->popClipRect();
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void drawText(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            std::string text = *((std::string*) q->args[1].vp);
            const carto::ScreenPos pos = *((const carto::ScreenPos*) q->args[2].vp);
            int maxWidth = q->args[3].i;
            bool breakLines = q->args[4].i == 1;
            self->drawText(text, pos, maxWidth, breakLines);
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void drawPolygon(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            const std::vector<carto::ScreenPos> poses = *((const std::vector<carto::ScreenPos>*) q->args[1].vp);
            self->drawPolygon(poses);
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void drawRoundRect(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            const carto::ScreenBounds rect = *((const carto::ScreenBounds*) q->args[1].vp);
            float radius = q->args[1].f;
            self->drawRoundRect(rect, radius);
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void drawBitmap(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            const carto::ScreenBounds rect = *((const carto::ScreenBounds*) q->args[1].vp);
            const std::shared_ptr<carto::Bitmap> bitmap = *((const std::shared_ptr<carto::Bitmap>*) q->args[2].vp);
            self->drawBitmap(rect, bitmap);
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void measureTextSize(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            std::string text = *((std::string*) q->args[1].vp);
            int maxWidth = q->args[2].i;
            bool breakLines = q->args[3].i == 1;
            carto::ScreenBounds bounds = self->measureTextSize(text, maxWidth, breakLines);
            q->returnValue.vp = (void*) new carto::ScreenBounds(bounds.getMin(), bounds.getMax());
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }

        static void buildBitmap(void *arg) {
            em_queued_call* q = (em_queued_call*)arg;
            carto::BitmapCanvas::EmscriptenImpl* self = (carto::BitmapCanvas::EmscriptenImpl*) q->args[0].vp;
            std::shared_ptr<carto::Bitmap> sharedBitmap = self->buildBitmap();
            carto::Bitmap* ptrBitmap = new Bitmap(
                sharedBitmap->getPixelDataPtr(), sharedBitmap->getWidth(), sharedBitmap->getHeight(), 
                sharedBitmap->getColorFormat(), sharedBitmap->getWidth() * -4
            );
            q->returnValue.vp = (void *) ptrBitmap;
            q->operationDone = 1;
            emscripten_futex_wake(&q->operationDone, INT_MAX);
        }
    };

    BitmapCanvas::EmscriptenImpl::EmscriptenImpl(int width, int height) :
        _width(width),
        _height(height),
        _drawMode(FILL),
        _strokeWidth(0),
        _color(),
        _font(),
        _fontSize(0),
        _lineHeight(1.3)
    {
        if (MapView::getRunOnMainThread()) {
            em_queued_call call = {EM_FUNC_SIG_V};
            call.args[0].vp = (void *)this;
            call.args[1].i = width;
            call.args[2].i = height;

            auto target_thread = emscripten_main_browser_thread_id();

            if (pthread_equal(target_thread, pthread_self())) {
                EmRunOnMainThread::constructor(&call);
            } else {
                emscripten_proxy_async(emscripten_proxy_get_system_queue(), target_thread, EmRunOnMainThread::constructor, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
            }

            return;
        }

        emscripten::val offscreenClass = emscripten::val::global("OffscreenCanvas");
        this->_canvas = offscreenClass.new_(width, height);
        this->_context = this->_canvas.call<emscripten::val>("getContext", std::string("2d"));
    }

    BitmapCanvas::EmscriptenImpl::~EmscriptenImpl() {
        if (MapView::getRunOnMainThread()) {
            auto target_thread = emscripten_main_browser_thread_id();

            if (!pthread_equal(target_thread, pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), target_thread, EmRunOnMainThread::destructor, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
            }
        }
    }

    void BitmapCanvas::EmscriptenImpl::setDrawMode(DrawMode mode) {
        _drawMode = mode;
    }

    void BitmapCanvas::EmscriptenImpl::setColor(const Color& color) {
        _color = "rgba(" + std::to_string(color.getR()) + "," + std::to_string(color.getG()) + "," + std::to_string(color.getB()) + "," + std::to_string(color.getA()/255) + ")";
    }

    void BitmapCanvas::EmscriptenImpl::setStrokeWidth(float width) {
        _strokeWidth = width;
    }

    void BitmapCanvas::EmscriptenImpl::setFont(const std::string& name, float size) {
        _font = name;
        _fontSize = static_cast<int>(size);
    }

    void BitmapCanvas::EmscriptenImpl::pushClipRect(const ScreenBounds& clipRect) {
        if (MapView::getRunOnMainThread()) {
            if (!pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;
                call.args[1].vp = (void *)&clipRect;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::pushClipRect, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
                return;
            }
        }

        this->_context.call<void>("save");

        auto rect = emscripten::val::global("Path2D").new_();
        rect.call<void, float, float, float, float>("rect", clipRect.getMin().getX(), clipRect.getMin().getY(), clipRect.getWidth(), clipRect.getHeight());
        _context.call<void>("clip", rect);
    }

    void BitmapCanvas::EmscriptenImpl::popClipRect() {
        if (MapView::getRunOnMainThread()) {
            if (!pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::popClipRect, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
                return;
            }
        }

        _context.call<void>("restore");
    }

    void BitmapCanvas::EmscriptenImpl::drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        if (MapView::getRunOnMainThread()) {
            if (!pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;
                call.args[1].vp = (void *)&text;
                call.args[2].vp = (void *)&pos;
                call.args[3].i = maxWidth;
                call.args[4].i = breakLines ? 1 : 0;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::drawText, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
                return;
            }
        }

        if (text.empty()) {
            return;
        }

        if (maxWidth < 0) {
            maxWidth = 65536;
        }

        std::vector<std::string> lines;
        std::vector<int> maxWidths;
        int maxAscent = 0;
        int maxDescent = 0;
        int baseHeight = 0;

        measureTextSizeAndEllipsizeText(text, maxWidth, breakLines, lines, maxWidths, maxAscent, maxDescent);

        switch (_drawMode) {
            case STROKE:
                _context.set("font", std::to_string(_fontSize) + "px " + _font);
                _context.set("lineWidth", _strokeWidth);
                _context.set("strokeStyle", _color);

                for (int i = 0; i < lines.size(); i += 1) {
                    _context.call<void>("strokeText", lines[i], pos.getX(), pos.getY() + baseHeight + maxAscent);
                    baseHeight += maxAscent + maxDescent;
                }
                break;
            case FILL:
                _context.set("font", std::to_string(_fontSize) + "px " + _font);
                _context.set("fillStyle", _color);
                for (int i = 0; i < lines.size(); i += 1) {
                    _context.call<void>("fillText", lines[i], pos.getX(), pos.getY() + baseHeight + maxAscent);
                    baseHeight += maxAscent + maxDescent;
                }
                break;
        }
    }

    void BitmapCanvas::EmscriptenImpl::drawRoundRect(const ScreenBounds& rect, float radius) {
        if (MapView::getRunOnMainThread()) {
            if (!pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;
                call.args[1].vp = (void *)&rect;
                call.args[2].f = radius;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::drawRoundRect, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
                return;
            }
        }

        float minX = round(rect.getMin().getX()), minY = round(rect.getMin().getY());
        float maxX = round(rect.getMax().getX()), maxY = round(rect.getMax().getY());
        float midX = round((minX + maxX) * 0.5f), midY = round((minY + maxY) * 0.5f);

        minX = std::max(minX, 0.0f);
        minY = std::max(minY, 0.0f);
        maxX = std::min(maxX, (float)_width - 1);
        maxY = std::min(maxY, (float)_height - 1);

        _context.call<void, int, int>("moveTo", minX, midY);
        _context.call<void, int, int, int, int, int>("arcTo", minX, minY, midX, minY, radius);
        _context.call<void, int, int, int, int, int>("arcTo", maxX, minY, maxX, midY, radius);
        _context.call<void, int, int, int, int, int>("arcTo", maxX, maxY, midX, maxY, radius);
        _context.call<void, int, int, int, int, int>("arcTo", minX, maxY, minX, midY, radius);

        _context.call<void>("closePath");

        switch (_drawMode) {
            case STROKE:
                _context.set("lineWidth", _strokeWidth);
                _context.set("strokeStyle", _color);
                _context.call<void>("stroke");
                break;
            case FILL:
                _context.set("fillStyle", _color);
                _context.call<void>("fill");
                break;
        }
    }

    void BitmapCanvas::EmscriptenImpl::drawPolygon(const std::vector<ScreenPos>& poses) {
        if (MapView::getRunOnMainThread()) {
            if (!pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;
                call.args[1].vp = (void *)&poses;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::drawPolygon, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
                return;
            }
        }

        if (poses.empty()) {
            return;
        }

        auto path = emscripten::val::global("Path2D").new_();
        path.call<void, double, double>("moveTo", poses[0].getX(), poses[0].getY());
        for (size_t i = 1; i < poses.size(); i++) {
            path.call<void, double, double>("lineTo", poses[i].getX(), poses[i].getY());
        }
        path.call<void>("closePath");
    
        switch (_drawMode) {
        case STROKE:
            _context.set("lineWidth", _strokeWidth);
            _context.set("strokeStyle", _color);
            _context.call<void>("stroke", path);
            break;
        case FILL:
            _context.set("fillStyle", _color);
            _context.call<void>("fill", path);
            break;
        }
    }

    void BitmapCanvas::EmscriptenImpl::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            return;
        }

        if (MapView::getRunOnMainThread()) {
            if (!pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;
                call.args[1].vp = (void *)&rect;
                call.args[2].vp = (void *)&bitmap;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::drawBitmap, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
                return;
            }
        }
        
        std::shared_ptr<Bitmap> rgbaBitmap = bitmap->getRGBABitmap();
        auto jsArray = emscripten::val::array(rgbaBitmap->getPixelData());
        auto jsUint8ClampedArray = emscripten::val::global("Uint8ClampedArray").new_(jsArray);
        auto imageData = emscripten::val::global("ImageData").new_(jsUint8ClampedArray, rgbaBitmap->getWidth(), rgbaBitmap->getHeight());
        emscripten::val imgCanvas;
        if (!MapView::getRunOnMainThread()) {
            imgCanvas = emscripten::val::global("OffscreenCanvas").new_(rgbaBitmap->getWidth(), rgbaBitmap->getHeight());
        } else {
            const emscripten::val document = emscripten::val::global("document");
            imgCanvas = document.call<emscripten::val>("createElement", std::string("canvas"));
            imgCanvas.set("width", rgbaBitmap->getWidth());
            imgCanvas.set("height", rgbaBitmap->getHeight());
        }
        auto imgContext = imgCanvas.call<emscripten::val, std::string>("getContext", "2d");
        imgContext.call<void>("putImageData", imageData, 0, 0);
        
        _context.call<void>("save");
        _context.call<void>("scale", 1, -1);
        _context.call<void>("drawImage", imgCanvas, rect.getMin().getX(), -rect.getMin().getY() - rect.getHeight(), rect.getWidth(), rect.getHeight());
        _context.call<void>("restore");
    }

    ScreenBounds BitmapCanvas::EmscriptenImpl::measureTextSize(std::string text, int maxWidth, bool breakLines) const {
        if (MapView::getRunOnMainThread()) {
            if (!pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;
                call.args[1].vp = (void *)&text;
                call.args[2].i = maxWidth;
                call.args[3].i = breakLines ? 1 : 0;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::measureTextSize, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
                std::shared_ptr<ScreenBounds> temp((ScreenBounds*) call.returnValue.vp);
                ScreenBounds output(temp->getMin(), temp->getMax());
                return output;
            }
        }

        std::vector<std::string> lines;
        std::vector<int> maxWidths;
        int maxAscent = 0;
        int maxDescent = 0;

        return measureTextSizeAndEllipsizeText(text, maxWidth, breakLines, lines, maxWidths, maxAscent, maxDescent);
    }

    ScreenBounds BitmapCanvas::EmscriptenImpl::measureTextSizeAndEllipsizeText(std::string &text, int maxWidth, bool breakLines, std::vector<std::string> &lines, std::vector<int> &maxWidths, int &maxAscent, int &maxDescent) const {
        if (text.empty()) {
            return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
        }

        if (maxWidth < 0) {
            maxWidth = 65536;
        }

        emscripten::val offscreenClass = emscripten::val::global("OffscreenCanvas");
        auto canvas = MapView::getRunOnMainThread() 
                        ? emscripten::val::global("document").call<emscripten::val>("createElement", std::string("canvas"))
                        : offscreenClass.new_(0, 0);
        auto context = canvas.call<emscripten::val>("getContext", std::string("2d"));

        context.set("font", std::to_string(_fontSize) + "px " + _font);
        if (_drawMode == STROKE) context.set("lineWidth", _strokeWidth);

        std::istringstream ss(text);
        std::string word;
        std::string line;
        std::vector<int> maxAscents;
        std::vector<int> maxDescents;

        while (ss >> word) {
            if (!lines.empty()) line = lines.back() + " " + word;
            else line = word;

            auto metrics = context.call<emscripten::val>("measureText", line);
            auto width = metrics["actualBoundingBoxRight"].as<int>() + metrics["actualBoundingBoxLeft"].as<int>();
            auto ascent = metrics["actualBoundingBoxAscent"].as<int>();
            auto descent = metrics["actualBoundingBoxDescent"].as<int>();

            if (width <= maxWidth) {
                if (!lines.empty()) {
                    lines[lines.size() - 1] = line;
                    maxWidths[maxWidths.size() - 1] = std::max(maxWidths.back(), width);
                    maxAscents[maxAscents.size() - 1] = std::max(maxAscents.back(), ascent);
                    maxDescents[maxDescents.size() - 1] = std::max(maxDescents.back(), descent);
                }
                else {
                    lines.emplace_back(line);
                    maxWidths.push_back(width);
                    maxAscents.push_back(ascent);
                    maxDescents.push_back(descent);
                }
            } else if (!breakLines) {
                for (int i = line.size() - 2; i >= 0; i -= 1) {
                    auto truncatedLine = line.substr(0, i) + "...";
                    auto metrics = context.call<emscripten::val>("measureText", truncatedLine);
                    auto width = metrics["actualBoundingBoxRight"].as<int>() + metrics["actualBoundingBoxLeft"].as<int>();
                    auto ascent = metrics["actualBoundingBoxAscent"].as<int>();
                    auto descent = metrics["actualBoundingBoxDescent"].as<int>();
                    if (width <= maxWidth) {
                        if (!lines.empty()) {
                            lines[lines.size() - 1] = truncatedLine;
                            maxWidths[maxWidths.size() - 1] = std::max(maxWidths.back(), width);
                            maxAscents[maxAscents.size() - 1] = std::max(maxAscents.back(), ascent);
                            maxDescents[maxDescents.size() - 1] = std::max(maxDescents.back(), descent);
                        }
                        else {
                            lines.emplace_back(truncatedLine);
                            maxWidths.push_back(width);
                            maxAscents.push_back(ascent);
                            maxDescents.push_back(descent);
                        }
                        break;
                    }
                }
                break;
            } else {
                lines.emplace_back(word);

                auto metrics = context.call<emscripten::val>("measureText", word);
                auto width = metrics["actualBoundingBoxRight"].as<int>() + metrics["actualBoundingBoxLeft"].as<int>();
                auto ascent = metrics["actualBoundingBoxAscent"].as<int>();
                auto descent = metrics["actualBoundingBoxDescent"].as<int>();

                maxWidths.push_back(width);
                maxAscents.push_back(ascent);
                maxDescents.push_back(descent);
            }
        }

        int lineMaxWidth = 0;
        for (const auto &val : maxWidths) {
            lineMaxWidth = std::max(lineMaxWidth, val);
        }

        maxAscent = 0;
        maxDescent = 0;
        for (const auto &val : maxAscents) {
            maxAscent = std::max(maxAscent, val);
        }
        for (const auto &val : maxDescents) {
            maxDescent = std::max(maxDescent, val);
        }

        maxAscent *= _lineHeight;
        maxDescent *= _lineHeight;
        int lineMaxHeight = maxAscent + maxDescent;

        return ScreenBounds(ScreenPos(0, 0), ScreenPos(lineMaxWidth, lineMaxHeight * lines.size()));
    }

    std::shared_ptr<Bitmap> BitmapCanvas::EmscriptenImpl::buildBitmap() const {
        if (MapView::getRunOnMainThread()) {
            if (!pthread_equal(emscripten_main_browser_thread_id(), pthread_self())) {
                em_queued_call call = {EM_FUNC_SIG_V};
                call.args[0].vp = (void *)this;

                emscripten_proxy_async(emscripten_proxy_get_system_queue(), emscripten_main_browser_thread_id(), EmRunOnMainThread::buildBitmap, &call);
                emscripten_wait_for_call_v(&call, INFINITY);
                std::shared_ptr<Bitmap> bitmap((Bitmap*) call.returnValue.vp);
                return bitmap;
            }
        }

        emscripten::val imageDataJS = _context.call<emscripten::val>("getImageData", 0, 0, _width, _height)["data"];
        std::vector<unsigned char> imageData = emscripten::convertJSArrayToNumberVector<unsigned char>(imageDataJS);
        auto bitmap = std::make_shared<Bitmap>(&imageData[0], _width, _height, ColorFormat::COLOR_FORMAT_RGBA, (_width * 4));
        return bitmap;
    }

    void BitmapCanvas::EmscriptenImpl::setCanvas(emscripten::val canvas) {
        _canvas = canvas;
    }

    void BitmapCanvas::EmscriptenImpl::setContext(emscripten::val context) {
        _context = context;
    }

    void BitmapCanvas::EmscriptenImpl::terminate() {
        this->_canvas = emscripten::val::undefined();
        this->_context = emscripten::val::undefined();
    }
}
