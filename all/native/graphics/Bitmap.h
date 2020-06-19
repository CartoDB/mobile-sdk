/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAP_H_
#define _CARTO_BITMAP_H_

#include <memory>
#include <string>
#include <vector>

namespace carto {
    class BinaryData;
    
    namespace ColorFormat {
        /**
         * Possible image formats.
         */
        enum ColorFormat {
            /**
             * Options for identifiny unsupported image formats.
             */
            COLOR_FORMAT_UNSUPPORTED = 0,
            /**
             * An image format that describes images with a single color channel.
             */
            COLOR_FORMAT_GRAYSCALE = 0x1909, //GL_LUMINANCE,
            /**
             * An image format that describes images with two channels, one for color and the other
             * for alpha.
             */
            COLOR_FORMAT_GRAYSCALE_ALPHA = 0x190A, //GL_LUMINANCE_ALPHA,
            /**
             * An image format that describes images with three channels, one for each color: red, green and blue.
             */
            COLOR_FORMAT_RGB = 0x1907, //GL_RGB,
            /**
             * An image format that describes images with four channels, one for each color: red, green and blue
             * and one for alpha.
             */
            COLOR_FORMAT_RGBA = 0x1908, //GL_RGBA,
            /**
             * An image format that describes images with four channels, one for each color: blue, green and red
             * and one for alpha. This color format will be converted to RGBA.
             */
            COLOR_FORMAT_BGRA = 1,
            /**
             * An image format that describes images with four channels, one for each color: red, green, and blue
             * and one for alpha. Each color is only four bits. This color format will be converted to RGBA.
             */
            COLOR_FORMAT_RGBA_4444 = 2,
            /**
             * An image format that describes images with three channels, one for each color: red, green, and blue.
             * Red and blue colors are each packed into 5 bits, green into 6 bits. This color format will be converted to RGB.
             */
            COLOR_FORMAT_RGB_565 = 3
        };
    }
    
    /**
     * A class that provides the functionality to store, compress, uncompress and resize basic image formats.
     * Currently supported formats are png and jpeg. Upon loading compressed images will be converted to
     * alpha premultiplied uncompressed bitmaps of various image formats.
     */
    class Bitmap {
    public:
        /**
         * Constructs a bitmap from an already decoded vector of bytes. The bitmap data is expected to be alpha premultiplied, if alpha channel is used.
         * If the power of two conversion flag is set, additional padding will be added to the image to make it's dimensions power of two.
         * This can be useful when creating OpenGL textures from the Bitmap, because some GPUs perform badly with non power of two textures.
         * @param pixelData A vector of decoded, premultiplied bitmap bytes.
         * @param width The width of the bitmap.
         * @param height The height of the bitmap.
         * @param colorFormat The color format of the bitmap.
         * @param bytesPerRow The total number of bytes per row. Some bitmaps have additional padding at the end of each row. If the value is negative, then bitmap is assumed to be vertically flipped. In this case absolute value of the bytesPerRow value is used.
         */
        Bitmap(const std::shared_ptr<BinaryData>& pixelData, unsigned int width, unsigned int height,
                   ColorFormat::ColorFormat colorFormat, int bytesPerRow);
        /**
         * Constructs a bitmap from an already decoded array of bytes. The bitmap data is expected to be alpha premultiplied.
         * If the power of two conversion flag is set, additional padding will be added to the image to make it's dimensions power of two.
         * This can be useful when creating OpenGL textures from the Bitmap, because some GPUs perform badly with non power of two textures.
         * @param pixelData The pointer to the beginning of an array containing the decoded, premultiplied bitmap bytes.
         * @param width The width of the bitmap.
         * @param height The height of the bitmap.
         * @param colorFormat The color format of the bitmap.
         * @param bytesPerRow The total number of bytes per row. Some bitmaps have additional padding at the end of each row.
         */
        Bitmap(const unsigned char* pixelData, unsigned int width, unsigned int height,
                   ColorFormat::ColorFormat colorFormat, unsigned int bytesPerRow);
        virtual ~Bitmap();
    
        /**
         * Returns the width of the bitmap.
         * @return The width of the bitmap.
         */
        unsigned int getWidth() const;
        /**
         * Returns the height of the bitmap.
         * @return The height of the bitmap.
         */
        unsigned int getHeight() const;
    
        /**
         * Returns the color format of this bitmap.
         * @return The color format of this bitmap.
         */
        ColorFormat::ColorFormat getColorFormat() const;
        /**
         * Returns the bytes per pixel parameter of this bitmap. Valid values are 1, 2, 3 and 4.
         * @return The bytes per pixel parameter of this bitmap.
         */
        unsigned int getBytesPerPixel() const;
    
        /**
         * Returns the pixel data of this bitmap.
         * @return A byte vector of the bitmap's pixel data.
         */
        const std::vector<unsigned char>& getPixelData() const;
        
        /**
         * Returns a copy of the pixel data of this bitmap.
         * @return A binary data of the bitmap's pixel data.
         */
        std::shared_ptr<BinaryData> getPixelDataPtr() const;
        
        /**
         * Compresses this bitmap to a png.
         * @return A byte vector of the png's data.
         * @deprecated Use compressToPNG.
         */
        std::shared_ptr<BinaryData> compressToPng() const;
    
        /**
         * Compresses this bitmap to a PNG format.
         * @return A byte vector of the PNG's data.
         */
        std::shared_ptr<BinaryData> compressToPNG() const;
    
        /**
         * Compresses this bitmap to a internal format.
         * This operation is intended for serialization of the data only, no actual compression is performed.
         * @return A byte vector of the serialized data.
         */
        std::shared_ptr<BinaryData> compressToInternal() const;
        
        /**
         * Returns resized version of the bitmap. The power of two padding added during the construction of this bitmap
         * will be removed prior to resizing. If the power of two conversion flag is set, new padding will be added to the image 
         * after resizing to make it's dimensions power of two.
         * @param width The new width of this bitmap.
         * @param height The new height of this bitmap.
         * @return The resized bitmap instance or null in case of error (wrong dimensions).
         */
        std::shared_ptr<Bitmap> getResizedBitmap(unsigned int width, unsigned int height) const;
         
        /**
         * Returns sub-bitmap with specified offsets and dimensions.
         * @param xOffset X coordinate offset in the bitmap.
         * @param yOffset Y coordinate offset in the bitmap.
         * @param width Width of the sub-bitmap.
         * @param height Height of the sub-bitmap.
         * @return Sub-bitmap instance or null in case of error (wrong dimensions).
         */
        std::shared_ptr<Bitmap> getSubBitmap(int xOffset, int yOffset, int width, int height) const;

        /**
         * Returns paddedsub-bitmap with specified offsets and dimensions.
         * @param xPadding Padding along X coordinate. If negative value is used, the bitmap is padded from the left. By default, bitmap is padded from the right.
         * @param yPadding Padding along Y coordinate. If negative value is used, the bitmap is padded from the top. By default, bitmap is padded from the bottom.
         * @return Padded bitmap instance or null in case of error (wrong dimensions).
         */
        std::shared_ptr<Bitmap> getPaddedBitmap(int xPadding, int yPadding) const;
        
        /**
         * Returns copy of the bitmap converted to RGBA format.
         * @return The bitmap with identical dimensions but converted to RGBA format.
         */
        std::shared_ptr<Bitmap> getRGBABitmap() const;
        
        /**
         * Creates a new bitmap from compressed byte vector.
         * If the power of two conversion flag is set, additional padding will be added to the image to make it's dimensions power of two.
         * @param compressedData The compressed bitmap data.
         * @return The bitmap created from the compressed data. If the decompression fails, null is returned.
         */
        static std::shared_ptr<Bitmap> CreateFromCompressed(const std::shared_ptr<BinaryData>& compressedData);
        /**
         * Creates a new bitmap from compressed byte data.
         * If the power of two conversion flag is set, additional padding will be added to the image to make it's dimensions power of two.
         * @param compressedData The compressed bitmap data.
         * @param dataSize size of the compressed data.
         * @return The bitmap created from the compressed data. If the decompression fails, null is returned.
         */
        static std::shared_ptr<Bitmap> CreateFromCompressed(const unsigned char* compressedData, std::size_t dataSize);
        
    protected:
        Bitmap();
        
        bool loadFromCompressedBytes(const unsigned char* compressedData, std::size_t dataSize);
        bool loadFromUncompressedBytes(const unsigned char* pixelData, unsigned int width, unsigned int height,
                                       ColorFormat::ColorFormat colorFormat, int bytesPerRow);
    
        static bool IsJPEG(const unsigned char* compressedData, std::size_t dataSize);
        static bool IsPNG(const unsigned char* compressedData, std::size_t dataSize);
        static bool IsWEBP(const unsigned char* compressedData, std::size_t dataSize);
        static bool IsNUTI(const unsigned char* compressedData, std::size_t dataSize);
    
        bool loadJPEG(const unsigned char* compressedData, std::size_t dataSize);
        bool loadPNG(const unsigned char* compressedData, std::size_t dataSize);
        bool loadWEBP(const unsigned char* compressedData, std::size_t dataSize);
        bool loadNUTI(const unsigned char* compressedData, std::size_t dataSize);
        
        unsigned int _width;
        unsigned int _height;
        unsigned int _bytesPerPixel;
        ColorFormat::ColorFormat _colorFormat;
    
        std::vector<unsigned char> _pixelData;
    };
    
}

#endif
