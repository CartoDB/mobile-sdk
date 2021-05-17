/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPCANVASUWPIMPL_H_
#define _CARTO_BITMAPCANVASUWPIMPL_H_

#include "graphics/BitmapCanvas.h"

#include <wrl.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_1helper.h>
#include <dwrite.h>
#include <dwrite_2.h>

namespace carto {

    class BitmapCanvas::UWPImpl : public BitmapCanvas::Impl {
    public:
        UWPImpl(int width, int height);
        virtual ~UWPImpl();

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
        HRESULT createDWriteTextLayout(const std::string& text, int maxWidth, bool breakLines, IDWriteTextLayout** pdwriteTextLayout) const;

        int _width;
        int _height;
        float _strokeWidth;
        DrawMode _drawMode;
        Microsoft::WRL::ComPtr<ID3D11Device> _device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
        Microsoft::WRL::ComPtr<ID2D1Device> _d2dDevice;
        Microsoft::WRL::ComPtr<ID2D1DeviceContext> _d2dContext;
        Microsoft::WRL::ComPtr<ID2D1Bitmap1> _d2dTargetBitmap;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> _d2dBrush;
        Microsoft::WRL::ComPtr<IDWriteFactory> _dwriteFactory;
        Microsoft::WRL::ComPtr<IDWriteTextFormat> _dwriteTextFormat;
    };

}

#endif
