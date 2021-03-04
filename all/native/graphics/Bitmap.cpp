#include "Bitmap.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include <stdext/zlib.h>

#include <jpeglib.h>
#include <png.h>
#include <webp/decode.h>

namespace {
    const unsigned char NUTiHeader[4] = { 'N', 'U', 'T', 'i' };

    struct LibPNGIOContainer {
        LibPNGIOContainer(const unsigned char* compressedDataPtr) : _compressedDataPtr(compressedDataPtr) { }
    
        const unsigned char* _compressedDataPtr;
    };

    void reportPNGErrorCallback(png_structp pngPtr, png_const_charp message) {
        carto::Log::Errorf("Bitmap: Error while loading PNG: %s", message);
    }    

    void reportPNGWarningCallback(png_structp pngPtr, png_const_charp message) {
        carto::Log::Warnf("Bitmap: Warning while loading PNG: %s", message);
    }    

    void readPNGCallback(png_structp pngPtr, png_bytep data, png_size_t length) {
        LibPNGIOContainer* ioContainer = static_cast<LibPNGIOContainer*>(png_get_io_ptr(pngPtr));
        for (std::size_t i = 0; i < length; i++) {
            data[i] = ioContainer->_compressedDataPtr[i];
        }
    
        ioContainer->_compressedDataPtr += length;
    }
    
    void writePNGCallback(png_structp pngPtr, png_bytep data, png_size_t length) {
        std::vector<unsigned char>* compressedData = static_cast<std::vector<unsigned char>* >(png_get_io_ptr(pngPtr));
        compressedData->reserve(compressedData->size() + length);
        for (std::size_t i = 0; i < length; i++) {
            compressedData->push_back(data[i]);
        }
    }
    
    struct JPEGErrorManager {
        struct jpeg_error_mgr pub;
    
        jmp_buf setjmp_buffer;
    };
    
    void JPEGErrorExit(j_common_ptr cinfo) {
        JPEGErrorManager* myErr = (JPEGErrorManager*) cinfo->err;
        longjmp(myErr->setjmp_buffer, 1);
    }
    
    template <typename T>
    void encodeInt(T data, unsigned char* buffer, std::size_t size) {
        for (std::size_t i = 0; i < size; i++) {
            buffer[i] = data & 255;
            data >>= 8;
        }
    }
        
    template <typename T>
    T decodeInt(const unsigned char* buffer, std::size_t size) {
        T data = 0;
        for (std::size_t i = 0; i < size; i++) {
            data <<= 8;
            data |= buffer[size - 1 - i];
        }
        return data;
    }

}

namespace carto {
    
    Bitmap::Bitmap(const std::shared_ptr<BinaryData>& pixelData, unsigned int width, unsigned int height,
                           ColorFormat::ColorFormat colorFormat, int bytesPerRow) :
        _width(0),
        _height(0),
        _bytesPerPixel(0),
        _colorFormat(ColorFormat::COLOR_FORMAT_UNSUPPORTED),
        _pixelData()
    {
        if (!pixelData) {
            throw NullArgumentException("Null pixelData");
        }

        loadFromUncompressedBytes(pixelData->data(), width, height, colorFormat, bytesPerRow);
    }
    
    Bitmap::Bitmap(const unsigned char* pixelData, unsigned int width, unsigned int height,
                           ColorFormat::ColorFormat colorFormat, unsigned int bytesPerRow) :
        _width(0),
        _height(0),
        _bytesPerPixel(0),
        _colorFormat(ColorFormat::COLOR_FORMAT_UNSUPPORTED),
        _pixelData()
    {
        if (!pixelData) {
            throw NullArgumentException("Null pixelData");
        }

        loadFromUncompressedBytes(pixelData, width, height, colorFormat, bytesPerRow);
    }
    
    Bitmap::~Bitmap() {
    }
    
    unsigned int Bitmap::getWidth() const {
        return _width;
    }
    
    unsigned int Bitmap::getHeight() const {
        return _height;
    }
    
    ColorFormat::ColorFormat Bitmap::getColorFormat() const {
        return _colorFormat;
    }
    
    unsigned int Bitmap::getBytesPerPixel() const {
        return _bytesPerPixel;
    }
    
    const std::vector<unsigned char>& Bitmap::getPixelData() const {
        return _pixelData;
    }
    
    std::shared_ptr<BinaryData> Bitmap::getPixelDataPtr() const {
        return std::make_shared<BinaryData>(_pixelData);
    }

    std::shared_ptr<BinaryData> Bitmap::compressToPng() const {
        return compressToPNG();
    }
    
    std::shared_ptr<BinaryData> Bitmap::compressToPNG() const {
        png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, reportPNGErrorCallback, reportPNGWarningCallback);
        if (!pngPtr) {
            Log::Error("Bitmap::compressToPng: Failed to compress bitmap to PNG");
            return std::shared_ptr<BinaryData>();
        }
    
        png_infop infoPtr = png_create_info_struct(pngPtr);
        if (!infoPtr) {
            png_destroy_write_struct(&pngPtr, NULL);
            Log::Error("Bitmap::compressToPng: Failed to compress bitmap to PNG");
            return std::shared_ptr<BinaryData>();
        }
    
        if (setjmp(png_jmpbuf(pngPtr))) {
            png_destroy_write_struct(&pngPtr, &infoPtr);
            Log::Error("Bitmap::compressToPng: Failed to compress bitmap to PNG");
            return std::shared_ptr<BinaryData>();
        }
    
        // Set callback for writing data
        std::vector<unsigned char> compressedData;
        png_set_write_fn(pngPtr, &compressedData, writePNGCallback, NULL);
    
        int colorType;
        bool unpremultiply = false;
        switch (_colorFormat) {
        case ColorFormat::COLOR_FORMAT_GRAYSCALE:
            colorType = PNG_COLOR_TYPE_GRAY;
            break;
        case ColorFormat::COLOR_FORMAT_GRAYSCALE_ALPHA:
            colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
            unpremultiply = true;
            break;
        case ColorFormat::COLOR_FORMAT_RGB:
            colorType = PNG_COLOR_TYPE_RGB;
            break;
        case ColorFormat::COLOR_FORMAT_RGBA:
            colorType = PNG_COLOR_TYPE_RGB_ALPHA;
            unpremultiply = true;
            break;
        default:
            png_destroy_write_struct(&pngPtr, &infoPtr);
            Log::Errorf("Bitmap::savePNG: Failed to compress bitmap to PNG, unsupported image format: %d", _colorFormat);
            return std::shared_ptr<BinaryData>();
        }
    
        // Set PNG info
        png_set_IHDR(pngPtr, infoPtr, _width, _height, 8, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
        // Write PNG info
        png_write_info(pngPtr, infoPtr);
    
        // Set row start pointers
        std::vector<png_bytep> rowPointers(_height);
    
        // Unpremultiply
        std::vector<unsigned char> pixelData = _pixelData;
        if (unpremultiply) {
            for (std::size_t i = 0; i < pixelData.size(); i += _bytesPerPixel) {
                unsigned int a = pixelData[i + _bytesPerPixel - 1];
                if (a == 0) {
                    continue;
                }
                for (unsigned int j = 0; j < _bytesPerPixel - 1; j++) {
                    unsigned int c = pixelData[i + j];
                    pixelData[i + j] = static_cast<unsigned char>(255 * c / a);
                }
            }
        }
    
        // Set the individual row pointers to point at the correct offsets of image data
        unsigned char* pixelDataPtr = pixelData.data();
        int bytesPerRealRow = _width * _bytesPerPixel;
        for (std::size_t i = 0; i < _height; i++) {
            rowPointers[_height - 1 - i] = pixelDataPtr + i * bytesPerRealRow;
        }
    
        // Write image
        png_write_image(pngPtr, rowPointers.data());
    
        // Write end
        png_write_end(pngPtr, infoPtr);
    
        // Free memory
        png_destroy_write_struct(&pngPtr, &infoPtr);
    
        return std::make_shared<BinaryData>(std::move(compressedData));
    }
    
    std::shared_ptr<BinaryData> Bitmap::compressToInternal() const {
        std::size_t size = sizeof(_width) + sizeof(_height) + sizeof(_bytesPerPixel) + sizeof(unsigned int) + _width * _height * _bytesPerPixel;
        std::vector<unsigned char> compressedData(sizeof(NUTiHeader) + size);
        std::copy(NUTiHeader, NUTiHeader + sizeof(NUTiHeader), compressedData.data());
        std::size_t offset = sizeof(NUTiHeader);
        encodeInt(_width, &compressedData.at(offset), sizeof(_width));
        offset += sizeof(_width);
        encodeInt(_height, &compressedData.at(offset), sizeof(_height));
        offset += sizeof(_height);
        encodeInt(_bytesPerPixel, &compressedData.at(offset), sizeof(_bytesPerPixel));
        offset += sizeof(_bytesPerPixel);
        encodeInt(static_cast<unsigned int>(_colorFormat), &compressedData.at(offset), sizeof(unsigned int));
        offset += sizeof(unsigned int);

        std::vector<unsigned char> pixelData(_width * _height * _bytesPerPixel);
        unsigned int bytesPerRow = _width * _bytesPerPixel;
        for (unsigned int i = 0; i < _height; i++) {
            const unsigned char* row = _pixelData.data() + i * bytesPerRow;
            std::copy(row, row + bytesPerRow, &pixelData[i * bytesPerRow]);
        }

        std::copy(pixelData.begin(), pixelData.end(), compressedData.begin() + offset);
        return std::make_shared<BinaryData>(std::move(compressedData));
    }
        
    std::shared_ptr<Bitmap> Bitmap::getResizedBitmap(unsigned int width, unsigned int height) const {
        if (width <= 0 || height <= 0) {
            return std::shared_ptr<Bitmap>();
        }

        // This will only scale the actual image part, the padding that was previously added to make the image
        // dimensions power of 2 will be ignored
        const unsigned char* dsrc = _pixelData.data();
        std::vector<unsigned char> pixelData(width * height * _bytesPerPixel);
        unsigned char* ddest = pixelData.data();
    
        bool bUpsampleX = (_width < width);
        bool bUpsampleY = (_height < height);
    
        // If too many input pixels map to one output pixel, our 32-bit accumulation values
        // could overflow - so, if we have huge mappings like that, cut down the weights:
        //    256 max color value
        //   *256 weight_x
        //   *256 weight_y
        //   *256 (16*16) maximum # of input pixels (x,y) - unless we cut the weights down...
        int weight_shift = 0;
        float source_texels_per_out_pixel = ((_width / static_cast<float>(width + 1))
                * (_height / static_cast<float>(height + 1)));
        float weight_per_pixel = source_texels_per_out_pixel * 256 * 256; //weight_x * weight_y
        float accum_per_pixel = weight_per_pixel * 256; //color value is 0-255
        float weight_div = accum_per_pixel / 4294967000.0f;
        if (weight_div > 1) {
            weight_shift = static_cast<int>(ceilf(logf(weight_div) / logf(2)));
        }
        weight_shift = std::min(15, weight_shift); // this could go to 15 and still be ok.
    
        float fh = 256 * _height / static_cast<float>(height);
        float fw = 256 * _width / static_cast<float>(width);
        // Cache x1a, x1b for all the columns
    
        std::vector<int> g_px1ab(width * 2 * 2);
    
        for (std::size_t x2 = 0; x2 < width; x2++) {
            // Find the x-range of input pixels that will contribute:
            int x1a = static_cast<int>((x2) * fw);
            int x1b = static_cast<int>((x2 + 1) * fw);
            if (bUpsampleX) {
                // Map to same pixel -> we want to interpolate between two pixels!
                x1b = x1a + 256;
            }
            x1b = std::min(x1b, static_cast<int>(256 * _width - 1));
            g_px1ab[x2 * 2 + 0] = x1a;
            g_px1ab[x2 * 2 + 1] = x1b;
        }
    
        // For every output pixel
        for (std::size_t y2 = 0; y2 < height; y2++) {
            // Find the y-range of input pixels that will contribute:
            int y1a = static_cast<int>((y2) * fh);
            int y1b = static_cast<int>((y2 + 1) * fh);
            if (bUpsampleY) {
                // Map to same pixel -> we want to interpolate between two pixels!
                y1b = y1a + 256;
            }
            y1b = std::min(y1b, static_cast<int>(256 * _height - 1));
            int y1c = y1a >> 8;
            int y1d = y1b >> 8;
    
            for (std::size_t x2 = 0; x2 < width; x2++) {
                // Find the x-range of input pixels that will contribute
                int x1a = g_px1ab[x2 * 2 + 0];
                int x1b = g_px1ab[x2 * 2 + 1];
                int x1c = x1a >> 8;
                int x1d = x1b >> 8;
    
                // Add ip all input pixels contributing to this output pixel
                unsigned int r = 0, g = 0, b = 0, a = 0, wa = 0;
                for (int y = y1c; y <= y1d; y++) {
                    unsigned int weight_y = 256;
                    if (y1c != y1d) {
                        if (y == y1c) {
                            weight_y = 256 - (y1a & 0xFF);
                        } else if (y == y1d) {
                            weight_y = (y1b & 0xFF);
                        }
                    }
    
                    const unsigned char* dsrc2 = &dsrc[y * _width * _bytesPerPixel + x1c * _bytesPerPixel];
                    for (int x = x1c; x <= x1d; x++) {
                        unsigned int weight_x = 256;
                        if (x1c != x1d) {
                            if (x == x1c) {
                                weight_x = 256 - (x1a & 0xFF);
                            } else if (x == x1d) {
                                weight_x = (x1b & 0xFF);
                            }
                        }
    
                        unsigned int w = (weight_x * weight_y) >> weight_shift;

                        unsigned char r_src = *dsrc2++;
                        r += r_src * w;
                        if (_bytesPerPixel > 1) {
                            unsigned char g_src = *dsrc2++;
                            g += g_src * w;
                        }
                        if (_bytesPerPixel > 2) {
                            unsigned char b_src = *dsrc2++;
                            b += b_src * w;
                        }
                        if (_bytesPerPixel > 3) {
                            unsigned char a_src = *dsrc2++;
                            a += a_src * w;
                        }
                        wa += w;
                    }
                }
                if (wa <= 0) {
                    wa = std::numeric_limits<int>::max();
                }
    
                // Write results
                *ddest++ = r / wa;
                if (_bytesPerPixel > 1) {
                    *ddest++ = g / wa;
                }
                if (_bytesPerPixel > 2) {
                    *ddest++ = b / wa;
                }
                if (_bytesPerPixel > 3) {
                    *ddest++ = a / wa;
                }
            }
        }
        
        return std::make_shared<Bitmap>(pixelData.data(), width, height, _colorFormat, -static_cast<int>(width * _bytesPerPixel));
    }
    
    std::shared_ptr<Bitmap> Bitmap::getSubBitmap(int xOffset, int yOffset, int width, int height) const {
        if (xOffset < 0 || yOffset < 0 || width <= 0 || height <= 0 || static_cast<unsigned int>(xOffset + width) > _width || static_cast<unsigned int>(yOffset + height) > _height) {
            return std::shared_ptr<Bitmap>();
        }
    
        std::vector<unsigned char> pixelData(width * height * _bytesPerPixel);
        for (int y = 0; y < height; y++) {
            const unsigned char* row = &_pixelData[((_height - 1 - y - yOffset) * _width + xOffset) * _bytesPerPixel];
            std::copy(row, row + width * _bytesPerPixel, &pixelData[y * width * _bytesPerPixel]);
        }
        return std::make_shared<Bitmap>(pixelData.data(), width, height, _colorFormat, static_cast<int>(width * _bytesPerPixel));
    }
    
    std::shared_ptr<Bitmap> Bitmap::getPaddedBitmap(int xPadding, int yPadding) const {
        int x0 = std::max(-xPadding, 0);
        int y0 = std::max(-yPadding, 0);
        unsigned int newWidth = _width + std::abs(xPadding);
        unsigned int newHeight = _height + std::abs(yPadding);
        std::vector<unsigned char> newPixelData(newWidth * newHeight * _bytesPerPixel, 0);
        for (unsigned int y = 0; y < _height; y++) {
            const unsigned char* row = &_pixelData[((_height - 1 - y) * _width) * _bytesPerPixel];
            std::copy(row, row + _width * _bytesPerPixel, &newPixelData[(x0 + (y + y0) * newWidth) * _bytesPerPixel]);
        }
        return std::make_shared<Bitmap>(newPixelData.data(), newWidth, newHeight, _colorFormat, static_cast<int>(newWidth * _bytesPerPixel));
    }
    
    std::shared_ptr<Bitmap> Bitmap::getRGBABitmap() const {
        std::vector<unsigned char> pixelData(_width * _height * 4, 255);
        for (unsigned int i = 0; i < _height; i++) {
            for (unsigned int j = 0; j < _width; j++) {
                unsigned int destIndex = (i * _width + j) * 4;
                unsigned int srcIndex = (i * _width + j) * _bytesPerPixel;
                switch (_colorFormat) {
                case ColorFormat::COLOR_FORMAT_GRAYSCALE:
                    pixelData[destIndex + 0] = _pixelData[srcIndex];
                    pixelData[destIndex + 1] = _pixelData[srcIndex];
                    pixelData[destIndex + 2] = _pixelData[srcIndex];
                    break;
                case ColorFormat::COLOR_FORMAT_GRAYSCALE_ALPHA:
                    pixelData[destIndex + 0] = _pixelData[srcIndex];
                    pixelData[destIndex + 1] = _pixelData[srcIndex];
                    pixelData[destIndex + 2] = _pixelData[srcIndex];
                    pixelData[destIndex + 3] = _pixelData[srcIndex + 1];
                    break;
                case ColorFormat::COLOR_FORMAT_RGB:
                    pixelData[destIndex + 0] = _pixelData[srcIndex + 0];
                    pixelData[destIndex + 1] = _pixelData[srcIndex + 1];
                    pixelData[destIndex + 2] = _pixelData[srcIndex + 2];
                    break;
                case ColorFormat::COLOR_FORMAT_RGBA:
                    pixelData[destIndex + 0] = _pixelData[srcIndex + 0];
                    pixelData[destIndex + 1] = _pixelData[srcIndex + 1];
                    pixelData[destIndex + 2] = _pixelData[srcIndex + 2];
                    pixelData[destIndex + 3] = _pixelData[srcIndex + 3];
                    break;
                case ColorFormat::COLOR_FORMAT_BGRA:
                    pixelData[destIndex + 0] = _pixelData[srcIndex + 2];
                    pixelData[destIndex + 1] = _pixelData[srcIndex + 1];
                    pixelData[destIndex + 2] = _pixelData[srcIndex + 0];
                    pixelData[destIndex + 3] = _pixelData[srcIndex + 3];
                    break;
                case ColorFormat::COLOR_FORMAT_RGBA_4444:
                    {
                        unsigned short color = *reinterpret_cast<const unsigned short*>(&_pixelData[srcIndex]);
                        unsigned char r = (color & 0xF000) >> 8;
                        r = r | (r >> 4);
                        unsigned char g = (color & 0xF00) >> 4;
                        g = g | (g >> 4);
                        unsigned char b = (color & 0xF0);
                        b = b | (b >> 4);
                        unsigned char a = (color & 0xF) << 4;
                        a = a | (a >> 4);
                        pixelData[destIndex + 0] = r;
                        pixelData[destIndex + 1] = g;
                        pixelData[destIndex + 2] = b;
                        pixelData[destIndex + 3] = a;
                    }
                    break;
                case ColorFormat::COLOR_FORMAT_RGB_565:
                    {
                        unsigned short color = *reinterpret_cast<const unsigned short*>(&_pixelData[srcIndex]);
                        unsigned char r = (color & 0xF800) >> 8;
                        r = r | (r >> 5);
                        unsigned char g = (color & 0x7E0) >> 3;
                        g = g | (g >> 6);
                        unsigned char b = (color & 0x1F) << 3;
                        b = b | (b >> 5);
                        pixelData[destIndex + 0] = r;
                        pixelData[destIndex + 1] = g;
                        pixelData[destIndex + 2] = b;
                    }
                    break;
                default:
                    Log::Error("Bitmap::getRGBABitmap: Failed to convert bitmap due to unsupported color format");
                    break;
                }
            }
        }
        
        // Create new bitmap
        return std::make_shared<Bitmap>(pixelData.data(), _width, _height, ColorFormat::COLOR_FORMAT_RGBA, -static_cast<int>(_width * 4));
    }
    
    std::shared_ptr<Bitmap> Bitmap::CreateFromCompressed(const std::shared_ptr<BinaryData>& compressedData) {
        if (!compressedData) {
            throw NullArgumentException("Null compressedData");
        }

        return CreateFromCompressed(compressedData->data(), compressedData->size());
    }

    std::shared_ptr<Bitmap> Bitmap::CreateFromCompressed(const unsigned char* compressedData, std::size_t dataSize) {
        if (!compressedData) {
            throw NullArgumentException("Null compressedData");
        }

        std::shared_ptr<Bitmap> bitmap(new Bitmap);
        if (!bitmap->loadFromCompressedBytes(compressedData, dataSize)) {
            return std::shared_ptr<Bitmap>();
        }
        return bitmap;
    }
    
    Bitmap::Bitmap() :
        _width(0),
        _height(0),
        _bytesPerPixel(0),
        _colorFormat(ColorFormat::COLOR_FORMAT_UNSUPPORTED),
        _pixelData()
    {
    }

    bool Bitmap::loadFromCompressedBytes(const unsigned char* compressedData, std::size_t dataSize) {
        if (IsJPEG(compressedData, dataSize)) {
            return loadJPEG(compressedData, dataSize);
        } else if (IsPNG(compressedData, dataSize)) {
            return loadPNG(compressedData, dataSize);
        } else if (IsWEBP(compressedData, dataSize)) {
            return loadWEBP(compressedData, dataSize);
        } else if (IsNUTI(compressedData, dataSize)) {
            return loadNUTI(compressedData, dataSize);
        } else {
            std::vector<unsigned char> uncompressedData;
            if (zlib::inflate_gzip(compressedData, dataSize, uncompressedData)) {
                Log::Info("Bitmap::loadFromCompressedBytes: Image is gzipped, decompressing");
                return loadFromCompressedBytes(uncompressedData.data(), uncompressedData.size());
            } else {
                Log::Error("Bitmap::loadFromCompressedBytes: Unsupported image format");
                return false;
            }
        }
    }
    
    bool Bitmap::loadFromUncompressedBytes(const unsigned char* pixelData, unsigned int width, unsigned int height, ColorFormat::ColorFormat colorFormat, int bytesPerRow) {
        _colorFormat = colorFormat;
        bool convert = false;
        switch (_colorFormat) {
        case ColorFormat::COLOR_FORMAT_GRAYSCALE:
            _bytesPerPixel = 1;
            break;
        case ColorFormat::COLOR_FORMAT_GRAYSCALE_ALPHA:
            _bytesPerPixel = 2;
            break;
        case ColorFormat::COLOR_FORMAT_RGB:
            _bytesPerPixel = 3;
            break;
        case ColorFormat::COLOR_FORMAT_RGBA:
            _bytesPerPixel = 4;
            break;
        case ColorFormat::COLOR_FORMAT_BGRA:
            _bytesPerPixel = 4;
            convert = true;
            break;
        case ColorFormat::COLOR_FORMAT_RGBA_4444:
            _bytesPerPixel = 2;
            convert = true;
            break;
        case ColorFormat::COLOR_FORMAT_RGB_565:
            _bytesPerPixel = 2;
            convert = true;
            break;
        default:
            Log::Errorf("Bitmap::loadFromUncompressedBytes: Failed to load bitmap, unsupported color format: %d", colorFormat);
            return false;
        }
    
        _width = width;
        _height = height;
            
        // Allocate space
        _pixelData.resize(_width * _height * _bytesPerPixel);
    
        // Copy data from pixelData to _pixelData
        unsigned int newBytesPerRow = _width * _bytesPerPixel;
        unsigned int newActualBytesPerRow = _width * _bytesPerPixel;
        
        if (convert) {
            for (unsigned int i = 0; i < _height; i++) {
                unsigned int flippedI = (_height - 1 - i);
                for (unsigned int j = 0; j < newActualBytesPerRow; j += _bytesPerPixel) {
                    unsigned int destIndex = flippedI * newBytesPerRow + j;
                    unsigned int srcIndex = (bytesPerRow < 0 ? flippedI : i) * std::abs(bytesPerRow) + j;
                    switch (_colorFormat) {
                    case ColorFormat::COLOR_FORMAT_BGRA:
                        _pixelData[destIndex] = pixelData[srcIndex + 2];
                        _pixelData[destIndex + 1] = pixelData[srcIndex + 1];
                        _pixelData[destIndex + 2] = pixelData[srcIndex];
                        _pixelData[destIndex + 3] = pixelData[srcIndex + 3];
                        break;
                    case ColorFormat::COLOR_FORMAT_RGBA_4444:
                        {
                            unsigned short color = *reinterpret_cast<const unsigned short*>(pixelData + srcIndex);
                            unsigned char r = (color & 0xF000) >> 8;
                            r = r | (r >> 4);
                            unsigned char g = (color & 0xF00) >> 4;
                            g = g | (g >> 4);
                            unsigned char b = (color & 0xF0);
                            b = b | (b >> 4);
                            unsigned char a = (color & 0xF) << 4;
                            a = a | (a >> 4);
                            _pixelData[destIndex] = r;
                            _pixelData[destIndex + 1] = g;
                            _pixelData[destIndex + 2] = b;
                            _pixelData[destIndex + 3] = a;
                        }
                        break;
                    case ColorFormat::COLOR_FORMAT_RGB_565:
                        {
                            unsigned short color = *reinterpret_cast<const unsigned short*>(pixelData + srcIndex);
                            unsigned char r = (color & 0xF800) >> 8;
                            r = r | (r >> 5);
                            unsigned char g = (color & 0x7E0) >> 3;
                            g = g | (g >> 6);
                            unsigned char b = (color & 0x1F) << 3;
                            b = b | (b >> 5);
                            _pixelData[destIndex] = r;
                            _pixelData[destIndex + 1] = g;
                            _pixelData[destIndex + 2] = b;
                        }
                        break;
                    default:
                        Log::Error("Bitmap::loadFromUncompressedBytes: Failed to load PNG, invalid color format for conversion");
                        break;
                    }
                }
            }
            switch (_colorFormat) {
            case ColorFormat::COLOR_FORMAT_BGRA:
            case ColorFormat::COLOR_FORMAT_RGBA_4444:
                _bytesPerPixel = 4;
                _colorFormat = ColorFormat::COLOR_FORMAT_RGBA;
                break;
            case ColorFormat::COLOR_FORMAT_RGB_565:
                _bytesPerPixel = 3;
                _colorFormat = ColorFormat::COLOR_FORMAT_RGB;
                break;
            default:
                Log::Error("Bitmap::loadFromUncompressedBytes: Failed to load PNG, invalid color format for conversion");
                break;
            }
        } else {
            // Normal copy
            for (unsigned int i = 0; i < _height; i++) {
                unsigned int flippedI = _height - 1 - i;
                unsigned int srcIndex = (bytesPerRow < 0 ? flippedI : i) * std::abs(bytesPerRow);
                for (unsigned int j = 0; j < newActualBytesPerRow; j++) {
                    _pixelData[flippedI * newBytesPerRow + j] = pixelData[srcIndex + j];
                }
            }
        }
        
        return true;
    }
    
    bool Bitmap::IsJPEG(const unsigned char* compressedData, std::size_t dataSize) {
        static const unsigned int JPG_SIGNATURE_LENGTH = 4;

        if (dataSize < JPG_SIGNATURE_LENGTH) {
            return false;
        }
        return compressedData[0] == 0xFF && compressedData[1] == 0xD8 && compressedData[2] == 0xFF && (compressedData[3] == 0xE0 || compressedData[3] == 0xE1);
    }
    
    bool Bitmap::IsPNG(const unsigned char* compressedData, std::size_t dataSize) {
        static const unsigned int PNG_SIGNATURE_LENGTH = 8;

        if (dataSize < PNG_SIGNATURE_LENGTH) {
            return false;
        }
        return png_sig_cmp(compressedData, 0, PNG_SIGNATURE_LENGTH) == 0;
    }
        
    bool Bitmap::IsWEBP(const unsigned char* compressedData, std::size_t dataSize) {
        static const unsigned int WEBP_SIGNATURE_LENGTH = 4;

        if (dataSize < WEBP_SIGNATURE_LENGTH) {
            return false;
        }
        return WebPGetInfo(compressedData, dataSize, nullptr, nullptr) == 1;
    }
    
    bool Bitmap::IsNUTI(const unsigned char* compressedData, std::size_t dataSize) {
        if (dataSize < sizeof(NUTiHeader)) {
            return false;
        }
        return std::equal(NUTiHeader, NUTiHeader + sizeof(NUTiHeader), compressedData);
    }
        
    bool Bitmap::loadJPEG(const unsigned char* compressedData, std::size_t dataSize) {
        jpeg_decompress_struct cinfo;
        JPEGErrorManager jerr;
        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = JPEGErrorExit;
    
        // Establish the setjmp return context for JPEGErrorExit to use
        if (setjmp(jerr.setjmp_buffer)) {
            jpeg_destroy_decompress(&cinfo);
            Log::Error("Bitmap::loadJPEG: Failed to load JPEG");
            return false;
        }
    
        // Create decompressing object, set data source
        jpeg_create_decompress(&cinfo);
        unsigned char* compressedDataPtr = const_cast<unsigned char*>(compressedData);
        jpeg_mem_src(&cinfo, compressedDataPtr, static_cast<unsigned long>(dataSize));
    
        // Read headers, prepare to decompress
        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);
    
        _width = cinfo.output_width;
        _height = cinfo.output_height;
    
        switch (cinfo.output_components) {
        case 1:
            _colorFormat = ColorFormat::COLOR_FORMAT_GRAYSCALE;
            break;
        case 3:
            _colorFormat = ColorFormat::COLOR_FORMAT_RGB;
            break;
        default:
            jpeg_destroy_decompress(&cinfo);
            Log::Error("Bitmap::loadJPEG: Failed to load JPEG, unsupported color format: %d, cinfo.output_components");
            break;
        }
    
        _bytesPerPixel = cinfo.output_components;
        int bytesPerRow = _width * _bytesPerPixel;
        _pixelData.resize(bytesPerRow * _height);
    
        // Read lines, flip y
        while (cinfo.output_scanline < _height) {
            unsigned char* pixelDataPtr = &_pixelData[(_height - 1 - cinfo.output_scanline) * bytesPerRow];
            jpeg_read_scanlines(&cinfo, &pixelDataPtr, 1);
        }
    
        // Finish and free the memory
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
    
        return true;
    }
    
    bool Bitmap::loadPNG(const unsigned char* compressedData, std::size_t dataSize) {
        png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, reportPNGErrorCallback, reportPNGWarningCallback);
        if (!pngPtr) {
            Log::Error("Bitmap::loadPNG: Failed to load PNG");
            return false;
        }
    
        png_infop infoPtr = png_create_info_struct(pngPtr);
        if (!infoPtr) {
            png_destroy_read_struct(&pngPtr, NULL, NULL);
            Log::Error("Bitmap::loadPNG: Failed to load PNG");
            return false;
        }
    
        png_infop endInfo = png_create_info_struct(pngPtr);
        if (!endInfo) {
            png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
            Log::Error("Bitmap::loadPNG: Failed to load PNG");
            return false;
        }
    
        if (setjmp(png_jmpbuf(pngPtr))) {
            png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
            Log::Error("Bitmap::loadPNG: Failed to load PNG");
            return false;
        }
    
        // Set callback method for reading data
        LibPNGIOContainer ioContainer(compressedData);
        png_set_read_fn(pngPtr, &ioContainer, readPNGCallback);
    
        // Read all the info up to the image data
        png_read_info(pngPtr, infoPtr);
    
        // Get info about png
        int colorType = 0;
        int bitDepth = 0;
        if (png_get_IHDR(pngPtr, infoPtr, &_width, &_height, &bitDepth, &colorType, NULL, NULL, NULL) == 0) {
            png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
            Log::Error("Bitmap::loadPNG: Failed to read PNG info");
            return false;
        }
    
        // Expand or strip images to 8 bit
        if (bitDepth == 1 || bitDepth == 2 || bitDepth == 4) {
            png_set_packing(pngPtr);
        } else if (bitDepth == 16) {
            png_set_strip_16(pngPtr);
        } else if (bitDepth != 8){
            png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
            Log::Errorf("Bitmap::loadPNG: Failed to load PNG, unsupported bit depth: %d", bitDepth);
            return false;
        }
    
        // Convert palette to rgb
        if (colorType == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(pngPtr);
        }
    
        // Convert tRNS to alpha
        if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)){
            png_set_tRNS_to_alpha(pngPtr);
        }
    
        // Update png info
        png_read_update_info(pngPtr, infoPtr);
        if (png_get_IHDR(pngPtr, infoPtr, &_width, &_height, &bitDepth, &colorType, NULL, NULL, NULL) == 0) {
            png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
            Log::Error("Bitmap::loadPNG: Failed to read PNG info");
            return false;
        }
    
        // Detect color type and necessity for alpha premultiplication
        bool premultiply = false;
        switch (colorType) {
        case PNG_COLOR_TYPE_GRAY:
            _colorFormat = ColorFormat::COLOR_FORMAT_GRAYSCALE;
            _bytesPerPixel = 1;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            _colorFormat = ColorFormat::COLOR_FORMAT_GRAYSCALE_ALPHA;
            _bytesPerPixel = 2;
            premultiply = true;
            break;
        case PNG_COLOR_TYPE_RGB:
            _colorFormat = ColorFormat::COLOR_FORMAT_RGB;
            _bytesPerPixel = 3;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            _colorFormat = ColorFormat::COLOR_FORMAT_RGBA;
            _bytesPerPixel = 4;
            premultiply = true;
            break;
        default:
            png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
            Log::Errorf("Bitmap::loadPNG: Failed to load PNG, unsupported color format: %d", colorType);
            return false;
        }
    
        int bytesPerRow = _width * _bytesPerPixel;
    
        // Allocate the image_data as a big block, to be given to opengl
        _pixelData.resize(bytesPerRow * _height);
        unsigned char* pixelDataPtr = _pixelData.data();
    
        // Set row start pointers
        std::vector<png_bytep> rowPointers(_height);
    
        // Set the individual row pointers to point at the correct offsets of image data
        for (std::size_t i = 0; i < _height; i++) {
            rowPointers[_height - 1 - i] = pixelDataPtr + i * bytesPerRow;
        }
    
        // Read the png into image_data through row_pointers
        png_read_image(pngPtr, rowPointers.data());
    
        if (premultiply) {
            // Premultiply alpha
            for (std::size_t i = 0; i < _pixelData.size(); i += _bytesPerPixel) {
                for (std::size_t j = 0; j < _bytesPerPixel - 1; j++) {
                    _pixelData[i + j] = (_pixelData[i + j] * _pixelData[i + _bytesPerPixel - 1]) / 255;
                }
            }
        }
    
        // Free memory
        png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
    
        return true;
    }
        
    bool Bitmap::loadWEBP(const unsigned char* compressedData, std::size_t dataSize) {
        WebPBitstreamFeatures features;
        if (WebPGetFeatures(compressedData, dataSize, &features) != VP8_STATUS_OK) {
            Log::Error("Bitmap::loadWEBP: Failed to load WEBP features");
            return false;
        }
        
        _width = features.width;
        _height = features.height;
    
        unsigned char* decodedData;
        if (features.has_alpha) {
            _bytesPerPixel = 4;
            _colorFormat = ColorFormat::COLOR_FORMAT_RGBA;
            decodedData = WebPDecodeRGBA(compressedData, dataSize, NULL, NULL);
        } else {
            _bytesPerPixel = 3;
            _colorFormat = ColorFormat::COLOR_FORMAT_RGB;
            decodedData = WebPDecodeRGB(compressedData, dataSize, NULL, NULL);
        }
        
        unsigned int bytesPerRow = _width * _bytesPerPixel;
        _pixelData.resize(_height * bytesPerRow);
        for (unsigned int i = 0; i < _height; i++) {
            for (unsigned int j = 0; j < bytesPerRow; j++) {
                _pixelData[(_height - i - 1) * bytesPerRow + j] = decodedData[i * bytesPerRow + j];
            }
        }
        
        WebPFree(decodedData);
        
        return true;
    }
    
    bool Bitmap::loadNUTI(const unsigned char* compressedData, std::size_t dataSize) {
        std::size_t offset = 4;
        _width = decodeInt<unsigned int>(&compressedData[offset], sizeof(_width));
        offset += sizeof(_width);
        _height = decodeInt<unsigned int>(&compressedData[offset], sizeof(_height));
        offset += sizeof(_height);
        _bytesPerPixel = decodeInt<unsigned int>(&compressedData[offset], sizeof(_bytesPerPixel));
        offset += sizeof(_bytesPerPixel);
        _colorFormat = static_cast<ColorFormat::ColorFormat>(decodeInt<unsigned int>(&compressedData[offset], sizeof(unsigned int)));
        offset += sizeof(unsigned int);
    
        unsigned int bytesPerRow = _width * _bytesPerPixel;
        _pixelData.resize(_height * bytesPerRow);
        for (unsigned int i = 0; i < _height; i++) {
            const unsigned char* row = &compressedData[offset + i * bytesPerRow];
            std::copy(row, row + bytesPerRow, &_pixelData[i * bytesPerRow]);
        }
    
        return true;
    }
        
}
