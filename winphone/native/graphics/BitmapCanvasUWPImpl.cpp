#include "graphics/BitmapCanvasUWPImpl.h"
#include "graphics/Bitmap.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <cmath>

#include <utf8.h>

namespace carto {

    BitmapCanvas::UWPImpl::UWPImpl(int width, int height) :
        _width(width),
        _height(height),
        _strokeWidth(0),
        _drawMode(FILL),
        _device(),
        _context(),
        _d2dDevice(),
        _d2dContext(),
        _d2dTargetBitmap(),
        _d2dBrush(),
        _dwriteFactory(),
        _dwriteTextFormat()
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
            &_device,
            NULL,
            &_context);
        if (SUCCEEDED(hr)) {
            Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
            _device.As(&dxgiDevice);

            hr = D2D1CreateDevice(dxgiDevice.Get(), NULL, &_d2dDevice);
            if (SUCCEEDED(hr)) {
                hr = _d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &_d2dContext);
                if (SUCCEEDED(hr)) {
                    D2D1_SIZE_U size = { static_cast<UINT32>(width), static_cast<UINT32>(height) };
                    D2D1_BITMAP_PROPERTIES1 targetProperties = { { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96, 96, D2D1_BITMAP_OPTIONS_TARGET, 0 };
                    hr = _d2dContext->CreateBitmap(size, NULL, 0, &targetProperties, &_d2dTargetBitmap);
                    if (SUCCEEDED(hr)) {
                        _d2dContext->SetTarget(_d2dTargetBitmap.Get());
                    } else {
                        throw GenericException("BitmapCanvas: Failed to create target bitmap");
                    }

                    _d2dContext->BeginDraw();
                    _d2dContext->Clear(D2D1::ColorF(0, 0));
                } else {
                    throw GenericException("BitmapCanvas: Failed to create D2DDeviceContext");
                }
            } else {
                throw GenericException("BitmapCanvas: Failed to create D2DDevice");
            }
        } else {
            throw GenericException("BitmapCanvas: Failed to create D3DDevice");
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &_dwriteFactory);
        if (FAILED(hr)) {
            throw GenericException("BitmapCanvas: Failed to create DWriteFactory");
        }
    }

    BitmapCanvas::UWPImpl::~UWPImpl() {
        if (_d2dContext) {
            _d2dContext->EndDraw();
        }
    }

    void BitmapCanvas::UWPImpl::setDrawMode(DrawMode mode) {
        _drawMode = mode;
    }

    void BitmapCanvas::UWPImpl::setColor(const Color& color) {
        if (_d2dContext) {
            _d2dContext->CreateSolidColorBrush(D2D1::ColorF(color.getARGB() & 0x00ffffff, static_cast<float>(color.getA()) / 255.0f), &_d2dBrush);
        }
    }

    void BitmapCanvas::UWPImpl::setStrokeWidth(float width) {
        _strokeWidth = width;
    }

    void BitmapCanvas::UWPImpl::setFont(const std::string& name, float size) {
        if (_dwriteFactory) {
            std::wstring wname;
            utf8::utf8to16(name.begin(), name.end(), std::back_inserter(wname));
            HRESULT hr = _dwriteFactory->CreateTextFormat(
                wname.c_str(),
                NULL,
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                size,
                L"",
                &_dwriteTextFormat
                );
            if (FAILED(hr)) {
                Log::Errorf("BitmapCanvas: Failed to create text format, %x", (int)hr);
            }
        }
    }

    void BitmapCanvas::UWPImpl::pushClipRect(const ScreenBounds& clipRect) {
        if (_d2dContext) {
            D2D1_RECT_F d2dRect = { clipRect.getMin().getX(), clipRect.getMin().getY(), clipRect.getMax().getX(), clipRect.getMax().getY() };
            _d2dContext->PushAxisAlignedClip(d2dRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
    }

    void BitmapCanvas::UWPImpl::popClipRect() {
        if (_d2dContext) {
            _d2dContext->PopAxisAlignedClip();
        }
    }

    void BitmapCanvas::UWPImpl::drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        if (text.empty()) {
            return;
        }

        if (_d2dContext && _dwriteFactory && _d2dBrush && _dwriteTextFormat) {
            Microsoft::WRL::ComPtr<IDWriteTextLayout> dwriteTextLayout;
            HRESULT hr = createDWriteTextLayout(text, maxWidth, breakLines, &dwriteTextLayout);
            if (SUCCEEDED(hr)) {
                D2D1_POINT_2F d2dPos = { pos.getX(), pos.getY() };
                _d2dContext->DrawTextLayout(d2dPos, dwriteTextLayout.Get(), _d2dBrush.Get());
            } else {
                Log::Errorf("BitmapCanvas: Failed to create text layout, %x", (int)hr);
            }
        }
    }

    void BitmapCanvas::UWPImpl::drawRoundRect(const ScreenBounds& rect, float radius) {
        if (_d2dContext && _d2dBrush) {
            D2D1_ROUNDED_RECT d2dRoundRect = { { rect.getMin().getX(), rect.getMin().getY(), rect.getMax().getX(), rect.getMax().getY() }, radius, radius };
            switch (_drawMode) {
            case STROKE:
                _d2dContext->DrawRoundedRectangle(d2dRoundRect, _d2dBrush.Get(), _strokeWidth);
                break;
            case FILL:
                _d2dContext->FillRoundedRectangle(d2dRoundRect, _d2dBrush.Get());
                break;
            }
        }
    }

    void BitmapCanvas::UWPImpl::drawPolygon(const std::vector<ScreenPos>& poses) {
        if (poses.empty()) {
            return;
        }

        if (_d2dContext && _d2dBrush) {
            Microsoft::WRL::ComPtr<ID2D1PathGeometry> d2dPathGeometry;
            Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory;
            _d2dDevice->GetFactory(&d2dFactory);
            HRESULT hr = d2dFactory->CreatePathGeometry(&d2dPathGeometry);
            if (SUCCEEDED(hr)) {
                Microsoft::WRL::ComPtr<ID2D1GeometrySink> d2dGeometrySink;
                d2dPathGeometry->Open(&d2dGeometrySink);
                d2dGeometrySink->BeginFigure(D2D1::Point2F(poses[0].getX(), poses[0].getY()), _drawMode == FILL ? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW);
                for (size_t i = 1; i < poses.size(); i++) {
                    d2dGeometrySink->AddLine(D2D1::Point2F(poses[i].getX(), poses[i].getY()));
                }
                d2dGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
                d2dGeometrySink->Close();
                
                switch (_drawMode) {
                case STROKE:
                    _d2dContext->DrawGeometry(d2dPathGeometry.Get(), _d2dBrush.Get(), _strokeWidth);
                    break;
                case FILL:
                    _d2dContext->FillGeometry(d2dPathGeometry.Get(), _d2dBrush.Get());
                    break;
                }
            } else {
                Log::Errorf("BitmapCanvas: Failed to create path geometry, %x", (int)hr);
            }
        }
    }

    void BitmapCanvas::UWPImpl::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            return;
        }

        if (_d2dContext) {
            if (bitmap->getWidth() < 1 || bitmap->getHeight() < 1) {
                return;
            }
            std::shared_ptr<Bitmap> rgbaBitmap = bitmap->getRGBABitmap();

            Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dBitmap;
            D2D1_BITMAP_PROPERTIES1 properties = { { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96, 96, D2D1_BITMAP_OPTIONS_NONE, 0 };
            D2D1_SIZE_U size = { rgbaBitmap->getWidth(), rgbaBitmap->getHeight() };
            HRESULT hr = _d2dContext->CreateBitmap(size, &rgbaBitmap->getPixelData()[0], rgbaBitmap->getWidth() * 4, &properties, &d2dBitmap);
            if (SUCCEEDED(hr)) {
                D2D1_RECT_F d2dRect = { rect.getMin().getX(), rect.getMin().getY(), rect.getMax().getX(), rect.getMax().getY() };
                _d2dContext->DrawBitmap(d2dBitmap.Get(), d2dRect);
            } else {
                Log::Errorf("BitmapCanvas: Failed to create bitmap, %x", (int)hr);
            }
        }
    }

    ScreenBounds BitmapCanvas::UWPImpl::measureTextSize(std::string text, int maxWidth, bool breakLines) const {
        if (text.empty()) {
            return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
        }

        if (_dwriteFactory && _dwriteTextFormat) {
            Microsoft::WRL::ComPtr<IDWriteTextLayout> dwriteTextLayout;
            HRESULT hr = createDWriteTextLayout(text, maxWidth, breakLines, &dwriteTextLayout);
            if (SUCCEEDED(hr)) {
                DWRITE_TEXT_METRICS dwriteMetrics;
                hr = dwriteTextLayout->GetMetrics(&dwriteMetrics);
                if (SUCCEEDED(hr)) {
                    return ScreenBounds(ScreenPos(0, 0), ScreenPos(std::ceil(dwriteMetrics.left + dwriteMetrics.width), std::ceil(dwriteMetrics.top + dwriteMetrics.height)));
                } else {
                    Log::Errorf("BitmapCanvas: Failed to read text metrics, %x", (int)hr);
                }
            } else {
                Log::Errorf("BitmapCanvas: Failed to create text layout, %x", (int)hr);
            }
        }
        return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
    }

    std::shared_ptr<Bitmap> BitmapCanvas::UWPImpl::buildBitmap() const {
        if (_d2dContext) {
            _d2dContext->EndDraw();

            Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dBitmap;
            D2D1_SIZE_U size = { static_cast<UINT32>(_width), static_cast<UINT32>(_height) };
            D2D1_BITMAP_PROPERTIES1 properties = { { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96, 96, D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ, 0 };
            HRESULT hr = _d2dContext->CreateBitmap(size, NULL, 0, &properties, &d2dBitmap);
            if (SUCCEEDED(hr)) {
                D2D1_POINT_2U point = { 0, 0 };
                D2D1_RECT_U rect = { 0, 0, static_cast<UINT32>(_width), static_cast<UINT32>(_height) };
                d2dBitmap->CopyFromBitmap(&point, _d2dTargetBitmap.Get(), &rect);

                D2D1_MAPPED_RECT mapped;
                hr = d2dBitmap->Map(D2D1_MAP_OPTIONS_READ, &mapped);
                if (SUCCEEDED(hr)) {
                    std::shared_ptr<Bitmap> bitmap = std::make_shared<Bitmap>(mapped.bits, _width, _height, ColorFormat::COLOR_FORMAT_RGBA, mapped.pitch);
                    d2dBitmap->Unmap();

                    _d2dContext->BeginDraw();
                    return bitmap;
                } else {
                    Log::Errorf("BitmapCanvas: Failed to map bitmap, %x", (int)hr);
                }
            } else {
                Log::Errorf("BitmapCanvas: Failed to create bitmap, %x", (int)hr);
            }

            _d2dContext->BeginDraw();
        }
        return std::shared_ptr<Bitmap>();
    }

    HRESULT BitmapCanvas::UWPImpl::createDWriteTextLayout(const std::string& text, int maxWidth, bool breakLines, IDWriteTextLayout** pdwriteTextLayout) const {
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
                static_cast<UINT32>(wtext.size()),
                _dwriteTextFormat.Get(),
                static_cast<FLOAT>(maxWidth < 0 ? 65536 : maxWidth),
                static_cast<FLOAT>(maxHeight),
                pdwriteTextLayout
                );
            if (SUCCEEDED(hr)) {
                Microsoft::WRL::ComPtr<IDWriteInlineObject> inlineObject;
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

}
