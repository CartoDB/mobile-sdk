#include "graphics/BitmapCanvas.h"
#include "graphics/Bitmap.h"
#include "utils/CFUniquePtr.h"
#include "utils/Log.h"

#include <cmath>
#include <vector>

#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>

namespace {
    CGSize measureFramesetter(CTFramesetterRef framesetter, CTFontRef font, int maxWidth, bool breakLines) {
        for (int extra = 0; extra < 65536; extra++) {
            CGFloat frameWidth = maxWidth < 0 ? CGFLOAT_MAX : maxWidth;
            CGFloat frameHeight = breakLines || maxWidth < 0 ? CGFLOAT_MAX : CTFontGetDescent(font) + CTFontGetAscent(font) + CTFontGetLeading(font) + extra;
            CFRange fitRange;
            CGSize size = CTFramesetterSuggestFrameSizeWithConstraints(framesetter, CFRangeMake(0, 0), nullptr, CGSizeMake(frameWidth, frameHeight), &fitRange);
            if (size.height > 0) {
                return size;
            }
        }
        return CGSizeMake(0, 0);
    }
}

namespace carto {
    
    struct BitmapCanvas::State {
        CFUniquePtr<CGColorSpaceRef> _colorSpace;
        CFUniquePtr<CGContextRef> _context;
        DrawMode _drawMode;
        float _strokeWidth;
        CFUniquePtr<CFNumberRef> _strokeWidthRef;
        CFUniquePtr<CGColorRef> _color;
        CFUniquePtr<CTFontRef> _font;
        float _fontSize;
        std::vector<unsigned char> _data;
        int _width;
        int _height;
        
        State(int width, int height) : _drawMode(FILL), _strokeWidth(0), _fontSize(0), _data(width * height * 4), _width(width), _height(height) { }
        
        CFAttributedStringRef createCFAttributedString(const std::string& text) {
            CFUniquePtr<CFMutableDictionaryRef> attributesDict(CFDictionaryCreateMutable(nullptr, 0, nullptr, nullptr));
            CFDictionaryAddValue(attributesDict, kCTFontAttributeName, _font);
            CFDictionaryAddValue(attributesDict, kCTForegroundColorAttributeName, _color);
            
            if (_drawMode == STROKE) {
                CFDictionaryAddValue(attributesDict, kCTStrokeColorAttributeName, _color);
                
                // We must keep a persistent reference to stroke width as dictionary values are not retained!
                int strokeWidth = (int)(-_strokeWidth / _fontSize * 100);
                _strokeWidthRef = CFUniquePtr<CFNumberRef>(CFNumberCreate(nullptr, kCFNumberIntType, &strokeWidth));
                CFDictionaryAddValue(attributesDict, kCTStrokeWidthAttributeName, _strokeWidthRef);
            }
 
            CFUniquePtr<CFStringRef> textString(CFStringCreateWithCString(nullptr, text.c_str(), kCFStringEncodingUTF8));
            return CFAttributedStringCreate(nullptr, textString, attributesDict);
        }
    };

    BitmapCanvas::BitmapCanvas(int width, int height) :
        _state(new State(width, height))
    {
        _state->_colorSpace = CFUniquePtr<CGColorSpaceRef>(CGColorSpaceCreateDeviceRGB(), CGColorSpaceRelease);
        if (width > 0 && height > 0) {
            _state->_context = CFUniquePtr<CGContextRef>(CGBitmapContextCreate(_state->_data.data(), width, height, 8, 4 * width, _state->_colorSpace, kCGImageAlphaPremultipliedLast), CGContextRelease);
        }
    }

    BitmapCanvas::~BitmapCanvas() {
    }

    void BitmapCanvas::setDrawMode(DrawMode mode) {
        _state->_drawMode = mode;
    }

    void BitmapCanvas::setColor(const Color& color) {
        CGFloat components[] = { color.getR() / 255.0f, color.getG() / 255.0f, color.getB() / 255.0f, color.getA() / 255.0f };
        CFUniquePtr<CGColorRef> baseColor(CGColorCreate(_state->_colorSpace, components), CGColorRelease);
        _state->_color = CFUniquePtr<CGColorRef>(CGColorCreateCopyWithAlpha(baseColor, color.getA() / 255.0f), CGColorRelease);
 	}

    void BitmapCanvas::setStrokeWidth(float width) {
        _state->_strokeWidth = width;
    }

    void BitmapCanvas::setFont(const std::string& name, float size) {
        CFUniquePtr<CFStringRef> nameRef(CFStringCreateWithCString(nullptr, name.c_str(), kCFStringEncodingUTF8));
        _state->_font = CFUniquePtr<CTFontRef>(CTFontCreateWithName(nameRef, size, nullptr));
        _state->_fontSize = size;
    }

    void BitmapCanvas::pushClipRect(const ScreenBounds& clipRect) {
        CGContextSaveGState(_state->_context);

        CGRect rect = CGRectMake(clipRect.getMin().getX(), clipRect.getMin().getY(), clipRect.getWidth(), clipRect.getHeight());
        CGContextClipToRect(_state->_context, rect);
    }

    void BitmapCanvas::popClipRect() {
        CGContextRestoreGState(_state->_context);
    }

    void BitmapCanvas::drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        CGContextRef context = _state->_context;
        CGContextSaveGState(context);
        
        // Flip the coordinate system
        CGContextSetTextMatrix(context, CGAffineTransformIdentity);
        CGContextTranslateCTM(context, 0, _state->_height);
        CGContextScaleCTM(context, 1.0, -1.0);
        
        CFUniquePtr<CFAttributedStringRef> attString(_state->createCFAttributedString(text));
        CFUniquePtr<CTFramesetterRef> framesetter(CTFramesetterCreateWithAttributedString(attString));
        CGSize frameSize = measureFramesetter(framesetter, _state->_font, maxWidth, breakLines);
        CGRect frameRect = CGRectMake(pos.getX(), pos.getY(), frameSize.width, frameSize.height);
        frameRect.origin.y = (_state->_height - frameRect.origin.y) - frameRect.size.height;
        CFUniquePtr<CGMutablePathRef> framePath(CGPathCreateMutable(), CGPathRelease);
        CGPathAddRect(framePath, nullptr, frameRect);
        CFUniquePtr<CTFrameRef> frame(CTFramesetterCreateFrame(framesetter, CFRangeMake(0, 0), framePath, nullptr));
        
        CFArrayRef lines = CTFrameGetLines(frame);
        CFIndex count = CFArrayGetCount(lines);
        std::vector<CGPoint> origins(count + 1);
        CTFrameGetLineOrigins(frame, CFRangeMake(0, count), origins.data());
        
        // note that we only enumerate to count-1 in here-- we draw the last line separately
        for (CFIndex i = 0; i < count - 1; i++) {
            // draw each line in the correct position as-is
            CGContextSetTextPosition(context, origins[i].x + frameRect.origin.x, origins[i].y + frameRect.origin.y);
            CTLineRef line = (CTLineRef)CFArrayGetValueAtIndex(lines, i);
            CTLineDraw(line, context);
        }
        
        // truncate the last line before drawing it
        if (count) {
            CGPoint lastOrigin = origins[count - 1];
            CTLineRef lastLine = (CTLineRef)CFArrayGetValueAtIndex(lines, count - 1);
            
            // truncation token is a CTLineRef itself
            CFRange effectiveRange;
            CFDictionaryRef stringAttrs = CFAttributedStringGetAttributes(attString, 0, &effectiveRange);
            
            CFUniquePtr<CFAttributedStringRef> truncationString(CFAttributedStringCreate(nullptr, CFSTR("\u2026"), stringAttrs));
            CFUniquePtr<CTLineRef> truncationToken(CTLineCreateWithAttributedString(truncationString));
            
            // now create the truncated line -- need to grab extra characters from the source string,
            // or else the system will see the line as already fitting within the given width and
            // will not truncate it.
            
            // range to cover everything from the start of lastLine to the end of the string
            CFRange rng = CFRangeMake(CTLineGetStringRange(lastLine).location, 0);
            rng.length = CFAttributedStringGetLength(attString) - rng.location;
            
            // substring with that range
            CFUniquePtr<CFAttributedStringRef> longString(CFAttributedStringCreateWithSubstring(nullptr, attString, rng));
            // line for that string
            CFUniquePtr<CTLineRef> longLine(CTLineCreateWithAttributedString(longString));
            
            CFUniquePtr<CTLineRef> truncatedLine(CTLineCreateTruncatedLine(longLine, std::ceil(frameRect.size.width), kCTLineTruncationEnd, truncationToken));
            
            // if 'truncated' is NULL, then no truncation was required to fit it
            if (!truncatedLine) {
                truncatedLine = CFUniquePtr<CTLineRef>((CTLineRef)CFRetain(lastLine));
            }
            
            // draw it at the same offset as the non-truncated version
            CGContextSetTextPosition(context, lastOrigin.x + frameRect.origin.x, lastOrigin.y + frameRect.origin.y);
            CTLineDraw(truncatedLine, context);
        }

        CGContextRestoreGState(context);
    }

    void BitmapCanvas::drawRoundRect(const ScreenBounds& rect, float radius) {

        float minX = rect.getMin().getX(), minY = rect.getMin().getY();
        float maxX = rect.getMax().getX(), maxY = rect.getMax().getY();
        float midX = (minX + maxX) * 0.5f, midY = (minY + maxY) * 0.5f;

        CGContextRef context = _state->_context;
        CGContextMoveToPoint(context, minX, midY);
        CGContextAddArcToPoint(context, minX, minY, midX, minY, radius);
        CGContextAddArcToPoint(context, maxX, minY, maxX, midY, radius);
        CGContextAddArcToPoint(context, maxX, maxY, midX, maxY, radius);
        CGContextAddArcToPoint(context, minX, maxY, minX, midY, radius);
        CGContextClosePath(context);
        
        switch (_state->_drawMode) {
            case STROKE:
                CGContextSetLineWidth(context, _state->_strokeWidth);
                CGContextSetStrokeColorWithColor(context, _state->_color);
                CGContextDrawPath(context, kCGPathStroke);
                break;
            case FILL:
                CGContextSetFillColorWithColor(context, _state->_color);
                CGContextDrawPath(context, kCGPathFill);
                break;
        }
    }
    
    void BitmapCanvas::drawPolygon(const std::vector<ScreenPos>& poses) {
        if (poses.empty()) {
            return;
        }

        CFUniquePtr<CGMutablePathRef> path(CGPathCreateMutable(), CGPathRelease);
        CGPathMoveToPoint(path, nullptr, poses[0].getX(), poses[0].getY());
        for (size_t i = 1; i < poses.size(); i++) {
            CGPathAddLineToPoint(path, nullptr, poses[i].getX(), poses[i].getY());
        }
        CGPathCloseSubpath(path);

        CGContextRef context = _state->_context;
        CGContextAddPath(context, path);
    
        switch (_state->_drawMode) {
        case STROKE:
            CGContextSetLineWidth(context, _state->_strokeWidth);
            CGContextSetStrokeColorWithColor(context, _state->_color);
            CGContextStrokePath(context);
            break;
        case FILL:
            CGContextSetFillColorWithColor(context, _state->_color);
            CGContextFillPath(context);
            break;
        }
    }

    void BitmapCanvas::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        if (!bitmap) {
            return;
        }

        CGContextRef context = _state->_context;
        
        std::shared_ptr<Bitmap> rgbaBitmap = bitmap->getRGBABitmap();
        CFUniquePtr<CGContextRef> bitmapContext(CGBitmapContextCreate(const_cast<unsigned char*>(rgbaBitmap->getPixelData().data()), rgbaBitmap->getWidth(), rgbaBitmap->getHeight(), 8, 4 * rgbaBitmap->getWidth(), _state->_colorSpace, kCGImageAlphaPremultipliedLast), CGContextRelease);
        CFUniquePtr<CGImageRef> bitmapImage(CGBitmapContextCreateImage(bitmapContext), CGImageRelease);
        
        CGContextDrawImage(context, CGRectMake(rect.getMin().getX(), rect.getMin().getY(), rect.getWidth(), rect.getHeight()), bitmapImage);
    }

    ScreenBounds BitmapCanvas::measureTextSize(std::string text, int maxWidth, bool breakLines) {
        CFUniquePtr<CFAttributedStringRef> attString(_state->createCFAttributedString(text));
        CFUniquePtr<CTFramesetterRef> framesetter(CTFramesetterCreateWithAttributedString(attString));
        CGSize frameSize = measureFramesetter(framesetter, _state->_font, maxWidth, breakLines);
        return ScreenBounds(ScreenPos(0, 0), ScreenPos(std::ceil(frameSize.width), std::ceil(frameSize.height)));
    }

    std::shared_ptr<Bitmap> BitmapCanvas::buildBitmap() const {
        return std::make_shared<Bitmap>(_state->_data.data(), _state->_width, _state->_height, ColorFormat::COLOR_FORMAT_RGBA, -(_state->_width * 4));
    }
}
