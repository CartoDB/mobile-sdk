/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CFUNIQUEPTR_H_
#define _CARTO_CFUNIQUEPTR_H_

#include <CoreFoundation/CoreFoundation.h>

namespace carto {

    template <typename T>
    struct CFUniquePtr {
        CFUniquePtr() = default;
        CFUniquePtr(const CFUniquePtr&) = delete;
        CFUniquePtr(CFUniquePtr&& other) { std::swap(_handle, other._handle); std::swap(_deleter, other._deleter); }
        explicit CFUniquePtr(T handle) : _handle(handle) { }
        template <typename S>
        explicit CFUniquePtr(T handle, void (*deleter)(S) = nullptr) : _handle(handle), _deleter(reinterpret_cast<void(*)(T)>(deleter)) { }
        ~CFUniquePtr() { if (_handle) { if (_deleter) (*_deleter)(_handle); else CFRelease(_handle); } }

        T get() const { return _handle; }
        operator T() const { return _handle; }
        CFUniquePtr& operator = (const CFUniquePtr&) = delete;
        CFUniquePtr& operator = (CFUniquePtr&& other) { std::swap(_handle, other._handle); std::swap(_deleter, other._deleter); return *this; }
           

    private:
        T _handle = NULL;
        void (*_deleter)(T) = nullptr;
    };

}

#endif
