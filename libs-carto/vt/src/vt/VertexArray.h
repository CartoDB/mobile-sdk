/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_VERTEXARRAY_H_
#define _CARTO_VT_VERTEXARRAY_H_

#include <utility>
#include <algorithm>

namespace carto { namespace vt {
    template <typename T>
    class VertexArray final {
    public:
        VertexArray() : _begin(nullptr), _end(nullptr), _reserved(0) { }

        VertexArray(VertexArray<T>&& other) : _begin(nullptr), _end(nullptr), _reserved(0) {
            std::swap(_begin, other._begin);
            std::swap(_end, other._end);
            std::swap(_reserved, other._reserved);
        }

        VertexArray(const VertexArray<T>&) = delete;
            
        ~VertexArray() {
            delete[] _begin;
        }

        const T* data() const { return _begin; }

        T* begin() { return _begin; }

        const T* begin() const { return _begin; }

        T* end() { return _end; }

        const T* end() const { return _end; }

        bool empty() const {
            return _begin == _end;
        }

        std::size_t size() const {
            return _end - _begin;
        }

        void clear() {
            _reserved += _end - _begin;
            _end = _begin;
        }

        void shrink_to_fit() {
            if (_reserved > 0) {
                reserve_extra(0, 1);
            }
        }

        void append(const T& val) {
            if (_reserved < 1) {
                reserve_extra(1, 2);
            }
            _end[0] = val;
            _end++;
            _reserved--;
        }

        void append(const T& val1, const T& val2) {
            if (_reserved < 2) {
                reserve_extra(2, 2);
            }
            _end[0] = val1;
            _end[1] = val2;
            _end += 2;
            _reserved -= 2;
        }

        void append(const T& val1, const T& val2, const T& val3) {
            if (_reserved < 3) {
                reserve_extra(3, 2);
            }
            _end[0] = val1;
            _end[1] = val2;
            _end[2] = val3;
            _end += 3;
            _reserved -= 3;
        }

        void append(const T& val1, const T& val2, const T& val3, const T& val4) {
            if (_reserved < 4) {
                reserve_extra(4, 2);
            }
            _end[0] = val1;
            _end[1] = val2;
            _end[2] = val3;
            _end[3] = val4;
            _end += 4;
            _reserved -= 4;
        }

        void copy(const VertexArray<T>& other, std::size_t offset, std::size_t size) {
            if (_reserved < size) {
                reserve_extra(size, empty() ? 1 : 2);
            }
            std::copy(other._begin + offset, other._begin + offset + size, _end);
            _end += size;
            _reserved -= size;
        }

        void fill(const T& val, std::size_t size) {
            if (_reserved < size) {
                reserve_extra(size, empty() ? 1 : 2);
            }
            std::fill(_end, _end + size, val);
            _end += size;
            _reserved -= size;
        }

        void alignTo(std::size_t alignment) {
            if (size() % alignment > 0) {
                fill(T(), alignment - size() % alignment);
            }
        }

        void reserve(std::size_t count) {
            reserve_extra(count, 1);
        }

        T& operator[] (std::size_t n) {
            return _begin[n];
        }

        const T& operator[] (std::size_t n) const {
            return _begin[n];
        }

        VertexArray<T>& operator = (VertexArray<T>&& other) {
            std::swap(_begin, other._begin);
            std::swap(_end, other._end);
            std::swap(_reserved, other._reserved);
            return *this;
        }

        VertexArray<T>& operator = (const VertexArray<T>&) = delete;

    private:
        void reserve_extra(std::size_t count, unsigned int multiplier) {
            std::size_t size = (_end - _begin + count) * multiplier;
            T* begin = nullptr;
            if (size > 0) {
                begin = new T[size];
                std::copy(_begin, _end, begin);
            }
            delete[] _begin;
            _reserved = size - (_end - _begin);
            _end = begin + (_end - _begin);
            _begin = begin;
        }

        T* _begin;
        T* _end;
        std::size_t _reserved; // number of elements reserved after _end
    };
} }

#endif
