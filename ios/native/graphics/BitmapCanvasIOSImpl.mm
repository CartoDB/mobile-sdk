#include "graphics/BitmapCanvasIOSImpl.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <cmath>
#include <vector>

#import <Foundation/Foundation.h>

namespace carto {
    
    BitmapCanvas::IOSImpl::IOSImpl(int width, int height) :
        _width(width),
        _height(height),
        _data(width * height * 4),
        _colorSpace(),
        _context(),
        _drawMode(FILL),
        _strokeWidth(0),
        _strokeWidthRef(),
        _color(),
        _font(),
        _fontSize(0)
    {
        _colorSpace = CFUniquePtr<CGColorSpaceRef>(CGColorSpaceCreateDeviceRGB(), CGColorSpaceRelease);
        if (width > 0 && height > 0) {
            _context = CFUniquePtr<CGContextRef>(CGBitmapContextCreate(_data.data(), width, height, 8, 4 * width, _colorSpace, kCGImageAlphaPremultipliedLast), CGContextRelease);
            if (!_context) {
                throw GenericException("Failed to create CGBitmap. Bitmap too large?");
            }
        }
    }

    BitmapCanvas::IOSImpl::~IOSImpl() {
    }

    void BitmapCanvas::IOSImpl::setDrawMode(DrawMode mode) {
        _drawMode = mode;
    }

    void BitmapCanvas::IOSImpl::setColor(const Color& color) {
        CGFloat components[] = { color.getR() / 255.0f, color.getG() / 255.0f, color.getB() / 255.0f, color.getA() / 255.0f };
        CFUniquePtr<CGColorRef> baseColor(CGColorCreate(_colorSpace, components), CGColorRelease);
        _color = CFUniquePtr<CGColorRef>(CGColorCreateCopyWithAlpha(baseColor, color.getA() / 255.0f), CGColorRelease);
    }

    void BitmapCanvas::IOSImpl::setStrokeWidth(float width) {
        _strokeWidth = width;
    }

    void BitmapCanvas::IOSImpl::setFont(const std::string& name, float size) {
        CFUniquePtr<CFStringRef> nameRef(CFStringCreateWithCString(nullptr, name.c_str(), kCFStringEncodingUTF8));
        _font = CFUniquePtr<CTFontRef>(CTFontCreateWithName(nameRef, size, nullptr));
        _fontSize = size;
    }

    void BitmapCanvas::IOSImpl::pushClipRect(const ScreenBounds& clipRect) {
        if (!_context) {
            return;
        }

        CGContextSaveGState(_context);

        CGRect rect = CGRectMake(clipRect.getMin().getX(), clipRect.getMin().getY(), clipRect.getWidth(), clipRect.getHeight());
        CGContextClipToRect(_context, rect);
    }

    void BitmapCanvas::IOSImpl::popClipRect() {
        if (!_context) {
            return;
        }

        CGContextRestoreGState(_context);
    }

    void BitmapCanvas::IOSImpl::drawText(std::string text, const ScreenPos& pos, int maxWidth, bool breakLines) {
        if (!_context) {
            return;
        }

        if (text.empty()) {
            return;
        }

        CGContextRef context = _context;
        CGContextSaveGState(context);
        
        // Flip the coordinate system
        CGContextSetTextMatrix(context, CGAffineTransformIdentity);
        CGContextTranslateCTM(context, 0, _height);
        CGContextScaleCTM(context, 1.0, -1.0);
        
        CFUniquePtr<CFAttributedStringRef> attString(createCFAttributedString(text));
        CFUniquePtr<CTFramesetterRef> framesetter(CTFramesetterCreateWithAttributedString(attString));
        CGSize frameSize = measureFramesetter(framesetter, maxWidth, breakLines);
        CGRect frameRect = CGRectMake(pos.getX(), pos.getY(), frameSize.width, frameSize.height);
        frameRect.origin.y = (_height - frameRect.origin.y) - frameRect.size.height;
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

    void BitmapCanvas::IOSImpl::drawRoundRect(const ScreenBounds& rect, float radius) {
        if (!_context) {
            return;
        }

        float minX = rect.getMin().getX(), minY = rect.getMin().getY();
        float maxX = rect.getMax().getX(), maxY = rect.getMax().getY();
        float midX = (minX + maxX) * 0.5f, midY = (minY + maxY) * 0.5f;

        CGContextRef context = _context;
        CGContextMoveToPoint(context, minX, midY);
        CGContextAddArcToPoint(context, minX, minY, midX, minY, radius);
        CGContextAddArcToPoint(context, maxX, minY, maxX, midY, radius);
        CGContextAddArcToPoint(context, maxX, maxY, midX, maxY, radius);
        CGContextAddArcToPoint(context, minX, maxY, minX, midY, radius);
        CGContextClosePath(context);
        
        switch (_drawMode) {
        case STROKE:
            CGContextSetLineWidth(context, _strokeWidth);
            CGContextSetStrokeColorWithColor(context, _color);
            CGContextDrawPath(context, kCGPathStroke);
            break;
        case FILL:
            CGContextSetFillColorWithColor(context, _color);
            CGContextDrawPath(context, kCGPathFill);
            break;
        }
    }
    
    void BitmapCanvas::IOSImpl::drawPolygon(const std::vector<ScreenPos>& poses) {
        if (!_context) {
            return;
        }

        if (poses.empty()) {
            return;
        }

        CFUniquePtr<CGMutablePathRef> path(CGPathCreateMutable(), CGPathRelease);
        CGPathMoveToPoint(path, nullptr, poses[0].getX(), poses[0].getY());
        for (size_t i = 1; i < poses.size(); i++) {
            CGPathAddLineToPoint(path, nullptr, poses[i].getX(), poses[i].getY());
        }
        CGPathCloseSubpath(path);

        CGContextRef context = _context;
        CGContextAddPath(context, path);
    
        switch (_drawMode) {
        case STROKE:
            CGContextSetLineWidth(context, _strokeWidth);
            CGContextSetStrokeColorWithColor(context, _color);
            CGContextStrokePath(context);
            break;
        case FILL:
            CGContextSetFillColorWithColor(context, _color);
            CGContextFillPath(context);
            break;
        }
    }

    void BitmapCanvas::IOSImpl::drawBitmap(const ScreenBounds& rect, const std::shared_ptr<Bitmap>& bitmap) {
        if (!_context) {
            return;
        }

        if (!bitmap) {
            return;
        }

        CGContextRef context = _context;
        
        std::shared_ptr<Bitmap> rgbaBitmap = bitmap->getRGBABitmap();
        CFUniquePtr<CGContextRef> bitmapContext(CGBitmapContextCreate(const_cast<unsigned char*>(rgbaBitmap->getPixelData().data()), rgbaBitmap->getWidth(), rgbaBitmap->getHeight(), 8, 4 * rgbaBitmap->getWidth(), _colorSpace, kCGImageAlphaPremultipliedLast), CGContextRelease);
        CFUniquePtr<CGImageRef> bitmapImage(CGBitmapContextCreateImage(bitmapContext), CGImageRelease);
        
        CGContextDrawImage(context, CGRectMake(rect.getMin().getX(), rect.getMin().getY(), rect.getWidth(), rect.getHeight()), bitmapImage);
    }

    ScreenBounds BitmapCanvas::IOSImpl::measureTextSize(std::string text, int maxWidth, bool breakLines) const {
        if (text.empty()) {
            return ScreenBounds(ScreenPos(0, 0), ScreenPos(0, 0));
        }

        CFUniquePtr<CFAttributedStringRef> attString(createCFAttributedString(text));
        CFUniquePtr<CTFramesetterRef> framesetter(CTFramesetterCreateWithAttributedString(attString));
        CGSize frameSize = measureFramesetter(framesetter, maxWidth, breakLines);
        return ScreenBounds(ScreenPos(0, 0), ScreenPos(std::ceil(frameSize.width), std::ceil(frameSize.height)));
    }

    std::shared_ptr<Bitmap> BitmapCanvas::IOSImpl::buildBitmap() const {
        return std::make_shared<Bitmap>(_data.data(), _width, _height, ColorFormat::COLOR_FORMAT_RGBA, -(_width * 4));
    }

    CFAttributedStringRef BitmapCanvas::IOSImpl::createCFAttributedString(const std::string& text) const {
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

    CGSize BitmapCanvas::IOSImpl::measureFramesetter(CTFramesetterRef framesetter, int maxWidth, bool breakLines) const {
        for (int extra = 0; extra < 65536; extra++) {
            CGFloat frameWidth = maxWidth < 0 ? CGFLOAT_MAX : maxWidth;
            CGFloat frameHeight = breakLines || maxWidth < 0 ? CGFLOAT_MAX : CTFontGetDescent(_font) + CTFontGetAscent(_font) + CTFontGetLeading(_font) + extra;
            CFRange fitRange;
            CGSize size = CTFramesetterSuggestFrameSizeWithConstraints(framesetter, CFRangeMake(0, 0), nullptr, CGSizeMake(frameWidth, frameHeight), &fitRange);
            if (size.height > 0) {
                return size;
            }
        }
        return CGSizeMake(0, 0);
    }

}
