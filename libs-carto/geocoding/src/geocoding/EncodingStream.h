/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_ENCODINGSTREAM_H_
#define _CARTO_GEOCODING_ENCODINGSTREAM_H_

#include <string>
#include <stdexcept>

#include <cglib/vec.h>

namespace carto { namespace geocoding {
    class EncodingStream final {
    public:
        explicit EncodingStream(const void* data, std::size_t size) : _data(reinterpret_cast<const unsigned char*>(data)), _size(size) { }

        bool eof() const { return _offset >= _size; }

        template <typename T>
        T readNumber() {
            long long num = 0;
            while (true) {
                if (_offset >= _size) {
                    throw std::runtime_error("Offset out of bounds");
                }
                unsigned char val = _data[_offset++];
                num = num + (val % 128);
                if (val < 128) {
                    break;
                }
                num = num * 128;
            }
            num = (num % 2 == 1 ? -(num + 1) / 2 : num / 2);
            return static_cast<T>(num);
        }

        template <typename T>
        T readDeltaNumber() {
            long long num = _prevNum + readNumber<long long>();
            _prevNum = num;
            return static_cast<T>(num);
        }

        cglib::vec2<double> readDeltaCoord(double scale) {
            long long x = _prevX + readNumber<long long>();
            long long y = _prevY + readNumber<long long>();
            _prevX = x;
            _prevY = y;
            return { static_cast<double>(x) * scale, static_cast<double>(y) * scale };
        }

        float readFloat() {
            if (_offset + 4 > _size) {
                throw std::runtime_error("Offset out of bounds");
            }
            std::uint32_t val = 0;
            for (int i = 0; i < 4; i++) {
                val = (val << 8) | _data[_offset++];
            }
            return *reinterpret_cast<float*>(&val);
        }

        std::string readString() {
            std::size_t offset = _offset;
            std::size_t len = readNumber<std::size_t>();
            if (offset + len > _size) {
                throw std::runtime_error("Offset out of bounds");
            }
            _offset += len;
            return std::string(reinterpret_cast<const char*>(_data + _offset - len), len);
        }

    private:
        long long _prevNum = 0;
        long long _prevX = 0;
        long long _prevY = 0;

        const unsigned char* _data = nullptr;
        std::size_t _offset = 0;
        std::size_t _size = 0;
    };
} }

#endif
