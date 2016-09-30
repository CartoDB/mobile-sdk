/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_COLOR_H_
#define _CARTO_COLOR_H_

#include <string>

namespace carto {

    /**
     * A color represented in RGBA space, alpha is expected to be non-premultiplied.
     */
    class Color {
    public:
        /**
         * Constructs a completely opaque black color object.
         */
        Color();
        /**
         * Constructs a map color object from the red, green, blue and alpha components.
         * Alpha component is interpreted as non-premultiplied transparency value,
         * thus to make half-transparent green, use Color(0, 255, 0, 128).
         * @param r The red component. Must be between 0 and 255.
         * @param g The green component. Must be between 0 and 255.
         * @param b The blue component. Must be between 0 and 255.
         * @param a The alpha component. Must be between 0 and 255.
         */
        Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
        /**
         * Constructs a map color object from a 32-bit encoded integer. The format is expected to be ARGB.
         * @param color The color encoded into 32-bit integer as ARGB.
         */
        explicit Color(int color);
    
        /**
         * Checks for equality between this and another map color.
         * @param color The other map vector.
         * @return True if equal.
         */
        bool operator ==(const Color& color) const;
        /**
         * Checks for inequality between this and another map color.
         * @param color The other map color.
         * @return True if not equal.
         */
        bool operator !=(const Color& color) const;
    
        /**
         * Returns the red component of this map color.
         * @return The red component in the [0..255] range.
         */
        unsigned char getR() const;
        /**
         * Returns the green component of this map color.
         * @return The green component in the [0..255] range.
         */
        unsigned char getG() const;
        /**
         * Returns the blue component of this map color.
         * @return The blue component in the [0..255] range.
         */
        unsigned char getB() const;
        /**
         * Returns the alpha component of this map color.
         * @return The alpha component in the [0..255] range.
         */
        unsigned char getA() const;
    
        /**
         * Encodes this map color into 32-bit integer value (ARGB format).
         * @return The encoded 32-bit integer representation of this map color.
         */
        int getARGB() const;
    
        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this map color, useful for logging.
         * @return The string representation of this map color.
         */
        std::string toString() const;
    
    private:
        unsigned char _r;
        unsigned char _g;
        unsigned char _b;
        unsigned char _a;
    };
    
}

#endif
