/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOCSS_COLOR_H_
#define _CARTO_CARTOCSS_COLOR_H_

#include <array>
#include <algorithm>
#include <functional>
#include <ostream>
#include <cmath>

namespace carto { namespace css {
    struct Color : private std::array<float, 4> {
        Color() : array() { }

        using array::begin;
        using array::end;
        using array::size;
        using array::operator [];

        unsigned int value() const {
            std::array<unsigned char, 4> components = rgba8();
            unsigned int val = components[3];
            for (std::size_t i = 0; i < 3; i++) {
                val = (val << 8) | components[i];
            }
            return val;
        }

        std::array<unsigned char, 4> rgba8() const {
            std::array<unsigned char, 4> components8;
            for (std::size_t i = 0; i < 4; i++) {
                float c = std::max(0.0f, std::min(1.0f, rgba()[i]));
                components8[i] = static_cast<unsigned char>(c * 255.0f + 0.5f);
            }
            return components8;
        }

        std::array<float, 4> rgba() const {
            return *this;
        }

        float alpha() const {
            return (*this)[3];
        }

        std::array<float, 4> hsla() const {
            float r = (*this)[0];
            float g = (*this)[1];
            float b = (*this)[2];
            float a = (*this)[3];

            float max = std::max(r, std::max(g, b));
            float min = std::min(r, std::min(g, b));
            float h = 0.0f;
            float s = 0.0f;
            float l = (max + min) * 0.5f;
            float d = max - min;
            if (max != min) {
                s = l > 0.5f ? d / (2.0f - max - min) : d / (max + min);
                if (max == r) {
                    h = (g - b) / d + (g < b ? 6.0f : 0.0f);
                }
                else if (max == g) {
                    h = (b - r) / d + 2.0f;
                }
                else {
                    h = (r - g) / d + 4.0f;
                }
                h /= 6.0f;
            }
            return std::array<float, 4>{{ h * 360.0f, s, l, a }};
        }

        static Color fromValue(unsigned int val) {
            std::array<unsigned char, 4> components;
            for (std::size_t i = 0; i < 3; i++) {
                components[2 - i] = static_cast<unsigned char>(val & 255);
                val >>= 8;
            }
            components[3] = static_cast<unsigned char>(val & 255);
            return fromRGBA8(components[0], components[1], components[2], components[3]);
        }

        static Color fromRGBA8(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
            return Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f);
        }

        static Color fromRGBA(float r, float g, float b, float a) {
            return Color(r, g, b, a);
        }

        static Color fromHSLA(float h, float s, float l, float a) {
            h = std::fmod(h, 360.0f) / 360.0f;
            if (h < 0.0f) {
                h += 1.0f;
            }
            return fromRGBA(hue(h + 1.0f / 3.0f, s, l), hue(h, s, l), hue(h - 1.0f / 3.0f, s, l), a);
        }

        static Color mix(Color color1, Color color2, float weight) {
            float w = weight * 2.0f - 1.0f;
            float da = color1.alpha() - color2.alpha();
            float w1 = (((w * da == -1.0f) ? w : (w + da) / (1.0f + w * da)) + 1.0f) * 0.5f;
            float w2 = weight;

            float r = color1.rgba()[0] * w1 + color2.rgba()[0] * (1.0f - w1);
            float g = color1.rgba()[1] * w1 + color2.rgba()[1] * (1.0f - w1);
            float b = color1.rgba()[2] * w1 + color2.rgba()[2] * (1.0f - w1);
            float a = color1.alpha()   * w2 + color2.alpha()   * (1.0f - w2);
            return Color::fromRGBA(r, g, b, a);
        }

        static Color spin(Color color, float amount) {
            std::array<float, 4> hsla = color.hsla();
            hsla[0] = hsla[0] + amount;
            return Color::fromHSLA(hsla[0], hsla[1], hsla[2], hsla[3]);
        }

        static Color saturate(Color color, float amount) {
            std::array<float, 4> hsla = color.hsla();
            hsla[1] = std::max(0.0f, std::min(1.0f, hsla[1] + amount));
            return Color::fromHSLA(hsla[0], hsla[1], hsla[2], hsla[3]);
        }

        static Color lighten(Color color, float amount) {
            std::array<float, 4> hsla = color.hsla();
            hsla[2] = std::max(0.0f, std::min(1.0f, hsla[2] + amount));
            return Color::fromHSLA(hsla[0], hsla[1], hsla[2], hsla[3]);
        }

        static Color fade(Color color, float amount) {
            std::array<float, 4> hsla = color.hsla();
            hsla[3] = std::max(0.0f, std::min(1.0f, hsla[3] + amount));
            return Color::fromHSLA(hsla[0], hsla[1], hsla[2], hsla[3]);
        }

    private:
        explicit Color(float r, float g, float b, float a) : array() {
            (*this)[0] = r;
            (*this)[1] = g;
            (*this)[2] = b;
            (*this)[3] = a;
        }
        
        static float hue(float h, float s, float l) {
            float m2 = (l <= 0.5f ? l * (s + 1.0f) : l + s - l * s);
            float m1 = l * 2.0f - m2;

            h = h < 0.0f ? h + 1.0f : (h > 1.0f ? h - 1.0f : h);
            if (h * 6.0f < 1.0f) {
                return m1 + (m2 - m1) * h * 6.0f;
            }
            else if (h * 2.0f < 1.0f) {
                return m2;
            }
            else if (h * 3.0f < 2.0f) {
                return m1 + (m2 - m1) * (2.0f / 3.0f - h) * 6.0f;
            }
            else {
                return m1;
            }
        }
    };

    inline Color operator + (const Color& color1, const Color& color2) {
        Color color;
        for (std::size_t i = 0; i < 3; i++) {
            color[i] = color1[i] + color2[i];
        }
        color[3] = 1.0f;
        return color;
    }

    inline Color operator - (const Color& color1, const Color& color2) {
        Color color;
        for (std::size_t i = 0; i < 3; i++) {
            color[i] = color1[i] - color2[i];
        }
        color[3] = 1.0f;
        return color;
    }

    inline Color operator * (const Color& color1, const Color& color2) {
        Color color;
        for (std::size_t i = 0; i < 3; i++) {
            color[i] = color1[i] * color2[i];
        }
        color[3] = 1.0f;
        return color;
    }

    inline Color operator * (const Color& color1, float c2) {
        Color color;
        for (std::size_t i = 0; i < 3; i++) {
            color[i] = color1[i] * c2;
        }
        color[3] = 1.0f;
        return color;
    }

    inline Color operator * (float c1, const Color& color2) {
        Color color;
        for (std::size_t i = 0; i < 3; i++) {
            color[i] = c1 * color2[i];
        }
        color[3] = 1.0f;
        return color;
    }

    inline Color operator / (const Color& color1, const Color& color2) {
        Color color;
        for (std::size_t i = 0; i < 3; i++) {
            color[i] = color1[i] / color2[i];
        }
        color[3] = 1.0f;
        return color;
    }

    inline Color operator / (const Color& color1, float c2) {
        Color color;
        for (std::size_t i = 0; i < 3; i++) {
            color[i] = color1[i] / c2;
        }
        color[3] = 1.0f;
        return color;
    }

    inline bool operator == (const Color& color1, const Color& color2) {
        return std::equal(color1.begin(), color1.end(), color2.begin());
    }
    
    inline bool operator != (const Color& color1, const Color& color2) {
        return !(color1 == color2);
    }

    inline bool operator < (const Color& color1, const Color& color2) {
        return std::lexicographical_compare(color1.begin(), color1.end(), color2.begin(), color2.end());
    }

    inline bool operator > (const Color& color1, const Color& color2) {
        return color2 < color1;
    }

    inline bool operator <= (const Color& color1, const Color& color2) {
        return !(color2 < color1);
    }

    inline bool operator >= (const Color& color1, const Color& color2) {
        return !(color1 < color2);
    }

    inline std::ostream& operator << (std::ostream& os, const Color& color) {
        if (color.rgba8()[3] == 255) {
            os << "rgb(";
            os << static_cast<int>(color.rgba8()[0]) << ",";
            os << static_cast<int>(color.rgba8()[1]) << ",";
            os << static_cast<int>(color.rgba8()[2]);
            os << ")";
        }
        else {
            os << "rgba(";
            os << static_cast<int>(color.rgba8()[0]) << ",";
            os << static_cast<int>(color.rgba8()[1]) << ",";
            os << static_cast<int>(color.rgba8()[2]) << ",";
            os << color.rgba()[3];
            os << ")";
        }
        return os;
    }
} }

#endif
