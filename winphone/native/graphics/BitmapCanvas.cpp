#include "graphics/BitmapCanvas.h"
#include "graphics/Bitmap.h"
#include "utils/Log.h"

#include <cmath>

#include <utf8.h>

#include <wrl.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <dwrite.h>
#include <d2d1_1helper.h>
#include <dwrite_2.h>

using namespace Microsoft::WRL;
using namespace D2D1;

namespace carto {

    struct BitmapCanvas::State {
        int _width;
        int _height;
        float _strokeWidth;
        DrawMode _drawMode;
        ComPtr<ID3D11Device> _device;
        ComPtr<ID3D11DeviceContext> _context;
        ComPtr<ID2D1Device> _d2dDevice;
        ComPtr<ID2D1DeviceContext> _d2dContext;
        ComPtr<ID2D1Bitmap1> _d2dTargetBitmap;
        ComPtr<ID2D1SolidColorBrush> _d2dBrush;
        ComPtr<IDWriteFactory> _dwriteFactory;
        ComPtr<IDWriteTextFormat> _dwriteTextFormat;

        State(int width, int height) : _width(width), _height(height), _strokeWidth(0), _drawMode(FILL) { }

        HRESULT createDWriteTextLayout(const std::string& text, int maxWidth, bool breakLines, IDWriteTextLayout** pdwriteTextLayout) const {
            HRESULT hr = S_OK;
            int maxHeight = 65536;
            std::wstring wtext;
            utf8::utf8to16(text.begin(), text.end(), std::back_inserter(wtext));

            for (int i = 0; i < 2; i++) {
                if (i == 1) {
                    DWRITE_TEXT_METRICS dwriteMetrics;
                    hr = (*pdwriteTextLayout)->GetMetrics(&dwriteMetrics);
                    if (SUCCEEDED(hr) && dwriteMetrics.lineCount > 0) {
                        maxHeight = static_cast<int>(std::ceil(dwriteMetrics.top + dwriteMetrics.height / dwriteMetrics.lineCount));
                    }

                    (*pdwriteTextLayout)->Release();
                    *pdwriteTextLayout = NULL;
                }
                
                hr = _dwriteFactory->CreateTextLayout(
                    wtext.c_str(),
                    wtext.size(),
                    _dwriteTextFormat.Get(),
                    static_cast<FLOAT>(maxWidth < 0 ? 65536 : maxWidth),
                    static_cast<FLOAT>(maxHeight),
                    pdwriteTextLayout
                    );
                if (SUCCEEDED(hr)) {
                    ComPtr<IDWriteInlineObject> inlineObject;
                    _dwriteFactory->CreateEllipsisTrimmingSign(
                        *pdwriteTextLayout,
                        &inlineObject);
                    DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
                    (*pdwriteTextLayout)->SetTrimming(&trimming, inlineObject.Get());
                }

                if (breakLines || maxWidth < 0) {
                    break;
                }
            }
            return hr;
        }
    };

    BitmapCanvas::BitmapCanvas(int width, int height) :
        _state(new State(width, height))
    {
        static const D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        HRESULT hr = D3D11CreateDevice(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &_state->_device,
            NULL,
            &_state->_context);
        if (SUCCEEDED(hr)) {
            ComPtr<IDXGIDevice> dxgiDevice;
            _state->_device.As(&dxgiDevice);

            hr = D2D1CreateDevice(dxgiDevice.Get(), NULL, &_state->_d2dDevice);
            if (SUCCEEDED(hr)) {
                hr = _state->_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &_state->_d2dContext);
                if (SUCCEEDED(hr)) {
                    D2D1_SIZE_U size = { width, height };
                    D2D1_BITMAP_PROPERTIES1 targetProperties = { { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96, 96, D2D1_BITMAP_OPTIONS_TARGET, 0 };
                    hr = _state->_d2dContext->CreateBitmap(size, NULL, 0, &targetProperties, &_state->_d2dTargetBitmap);
                    if (SUCCEEDED(hr)) {
                        _state->_d2dContext->SetTarget(_state->_d2dTargetBitmap.Get());
                    }
                    else {
                        Log::Errorf("BitmapCanvas: Failed to create target bitmap, %x", (int)hr);
                    }

                    _state->_d2dContext->BeginDraw();
                    _state->_d2dContext->Clear(D2D1::ColorF(0, 0));
                }
                else {
                    Log::Errorf("BitmapCanvas: Failed to create D2DDeviceContext, %x", (int)hr);
                }
            }
            else {
                Log::Errorf("BitmapCanvas: Failed to create D2DDevice, %x", (int)hr);
            }
        }
        else {
            Log::Errorf("BitmapCanvas: Failed to create D3DDevice, %x", (int)hr);
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &_state->_dwriteFactory);
        if (FAILED(hr)) {
            Log::Errorf("BitmapCanvas: Failed to create DWriteFactory, %x", (int)hr);
        }
    }

    BitmapCanvas::~BitmapCanvas() {
        if (_state->_d2dContext) {
            _state->_d2dContext->EndDraw();
        }
    }

    void BitmapCanvas::setDrawMode(DrawMode mode) {
        _state->_drawMode = mode;
    }

    void BitmapCanvas::setColor(const Color& color) {
        if (_state->_d2dContext) {
            _state->_d2dContext->CreateSolidColorBrush(D2D1::ColorF(color.getARGB() & 0x00ffffff, static_cast<float>(color.getA()) / 255.0f), &_state->_d2dBrush);
        }
    }

    void BitmapCanvas::setStrokeWidth(float width) {
        _state->_strokeWidth = width;
    }

    void BitmapCanvas::setFont(const std::string& name, float size) {
        if (_state->_dwriteFactory) {
            std::wstring wname;
            utf8::utf8to16(name.begin(), name.end(), std::back_inserter(wname));
            HRESULT hr = _state->_dwriteFactory->CreateTextFormat(
                wname.c_str(),
                NULL,
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                size,
                L"",
                &_state->_dwriteTextFormat
                );
            if (FAILED(hr)) {
                Log::Errorf("BitmapCanvas: Failed to create text format, %x", (int)hr);
            }
        }
    }

    void BitmapCanvas::pushClipRect(const ScreenBounds& clipRect) {
        if (_state->_d2dContext) {
            D2D1_RECT_F d2dRect = { clipRect.getMin().getX(), clipRect.getMin().getY(), clipRect.getMax().getX(), clipRect.getMax().getY() };
            _state->_d2dContext->PushAxisAlignedClip(d2dRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
    }

    void BitmapCanvas::popClipRect() {
        if (_state->_d2dContext) {
            _state->_d2dContext->PopAxisAlignedClip();
        }
    }

    void BitmapCanvas::drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        if (_state->_d2dContext && _state->_dwriteFactory && _state->_d2dBrush && _state->_dwriteTextFormat) {
            ComPtr<IDWriteTextLayout> dwriteTextLayout;
            HRESULT hr = _state->createDWriteTextLayout(text, maxWidth, breakLines, &dwriteTextLayout);
            if (SUCCEEDED(hr)) {
                D2D1_POINT_2F d2dPos = { pos.getX(), pos.getY() };
                _state->_d2dContext->DrawTextLayout(d2dPos, dwriteTextLayout.Get(), _state->_d2dBrush.Get());
            }
            else {
                Log::Errorf("BitmapCanvas: Failed to create text layout, %x", (int)hr);
            }
        }
    }

    void BitmapCanvas::drawRoundRect(const ScreenBounds& rect, float radius) {
        if (_state->_d2dContext && _state->_d2dBrush) {
            D2D1_ROUNDED_RECT d2dRoundRect = { { rect.getMin().getX(), rect.getMin().getY(), rect.getMax().getX(), rect.getMax().getY() }, radius, radius };
            switch (_state->_drawMode) {
            case STROKE:
                _state->_d2dContext->DrawRoundedRectangle(d2dRoundRect, _state->_d2dBrush.Get(), _state->_strokeWidth);
                break;
            case FILL:
                _state->_d2dContext->FillRoundedRectangle(d2dRoundRect, _state->_d2dBrush.Get());
                break;
            }
        }
    }

    void BitmapCanvas::drawPolygon(const std::vector<ScreenPos>& poses) {
        if (poses.empty()) {
            return;
        }

        if (_state->_d2dContext && _state->_d2dBrush) {
            ComPtr<ID2D1PathGeometry> d2dPathGeometry;
            ComPtr<ID2D1Factory> d2dFactory;
            _state->_d2dDevice->GetFactory(&d2dFactory);
            HRESULT hr = d2dFactory->CreatePathGeometry(&d2dPathGeometry);
            if (SUCCEEDED(hr)) {
                ComPtr<ID2D1GeometrySink> d2dGeometrySink;
                d2dPathGeometry->Open(&d2dGeometrySink);
                d2dGeometrySink->BeginFigure(Point2F(poses[0].getX(), poses[0].getY()), _state->_drawMode == FILL ? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW);
                for (size_t i = 1; i < poses.size(); i++) {
                    d2dGeometrySink->AddLine(Point2F(poses[i].getX(), poses[i].getY()));
                }
                d2dGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
                d2dGeometrySink->Close();
                
                switch (_state->_drawMode) {
                case STROKE:
                    _state->_d2dContext->DrawGeometry(d2dPathGeometry.Get(), _state->_d2dBrush.Get(), _state->_strokeWidth);
                    break;
                case FILL:
                    _state->_d2dContext->FillGeometry(d2dPathGeometry.Get(), _state->_d2dBrush.Get());
                    break;
                }
            }
            else {
                Log::Errorf("BitmapCanvas: Failed to create path geometry, %x", (int)hr);
            }
        }
    }

    void BitmapCanvas::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            return;
        }

        if (_state->_d2dContext) {
            if (bitmap->getWidth() < 1 || bitmap->getHeight() < 1) {
                return;
            }
            std::shared_ptr<Bitmap> rgbaBitmap = bitmap->getRGBABitmap();

            ComPtr<ID2D1Bitmap1> d2dBitmap;
            D2D1_BITMAP_PROPERTIES1 properties = { { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96, 96, D2D1_BITMAP_OPTIONS_NONE, 0 };
            D2D1_SIZE_U size = { rgbaBitmap->getWidth(), rgbaBitmap->getHeight() };
            HRESULT hr = _state->_d2dContext->CreateBitmap(size, &rgbaBitmap->getPixelData()[0], rgbaBitmap->getWidth() * 4, &properties, &d2dBitmap);
            if (SUCCEEDED(hr)) {
                D2D1_RECT_F d2dRect = { rect.getMin().getX(), rect.getMin().getY(), rect.getMax().getX(), rect.getMax().getY() };
                _state->_d2dContext->DrawBitmap(d2dBitmap.Get(), d2dRect);
            }
            else {
                Log::Errorf("BitmapCanvas: Failed to create bitmap, %x", (int)hr);
            }
        }
    }

    ScreenBounds BitmapCanvas::measureTextSize(std::string text, int maxWidth, bool breakLines) {
        if (_state->_dwriteFactory && _state->_dwriteTextFormat) {
            ComPtr<IDWriteTextLayout> dwriteTextLayout;
            HRESULT hr = _state->createDWriteTextLayout(text, maxWidth, breakLines, &dwriteTextLayout);
            if (SUCCEEDED(hr)) {
                DWRITE_TEXT_METRICS dwriteMetrics;
                hr = dwriteTextLayout->GetMetrics(&dwriteMetrics);
                if (SUCCEEDED(hr)) {
                    return ScreenBounds(ScreenPos(0, 0), ScreenPos(std::ceil(dwriteMetrics.left + dwriteMetrics.width), std::ceil(dwriteMetrics.top + dwriteMetrics.height)));
                }
                else {
                    Log::Errorf("BitmapCanvas: Failed to read text metrics, %x", (int)hr);
                }
            }
            else {
                Log::Errorf("BitmapCanvas: Failed to create text layout, %x", (int)hr);
            }
        }
        return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
    }

    std::shared_ptr<Bitmap> BitmapCanvas::buildBitmap() const {
        if (_state->_d2dContext) {
            _state->_d2dContext->EndDraw();

            ComPtr<ID2D1Bitmap1> d2dBitmap;
            D2D1_SIZE_U size = { _state->_width, _state->_height };
            D2D1_BITMAP_PROPERTIES1 properties = { { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96, 96, D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ, 0 };
            HRESULT hr = _state->_d2dContext->CreateBitmap(size, NULL, 0, &properties, &d2dBitmap);
            if (SUCCEEDED(hr)) {
                D2D1_POINT_2U point = { 0, 0 };
                D2D1_RECT_U rect = { 0, 0, _state->_width, _state->_height };
                d2dBitmap->CopyFromBitmap(&point, _state->_d2dTargetBitmap.Get(), &rect);

                D2D1_MAPPED_RECT mapped;
                hr = d2dBitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped);
                if (SUCCEEDED(hr)) {
                    std::shared_ptr<Bitmap> bitmap = std::make_shared<Bitmap>(mapped.bits, _state->_width, _state->_height, ColorFormat::COLOR_FORMAT_RGBA, mapped.pitch);
                    d2dBitmap->Unmap();
                    return bitmap;
                }
                else {
                    Log::Errorf("BitmapCanvas: Failed to map bitmap, %x", (int)hr);
                }
            }
            else {
                Log::Errorf("BitmapCanvas: Failed to create bitmap, %x", (int)hr);
            }

            _state->_d2dContext->BeginDraw();
        }
        return std::shared_ptr<Bitmap>();
    }

}
