/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_JNIUNIQUEGLOBALREF_H_
#define _CARTO_JNIUNIQUEGLOBALREF_H_

#include <jni.h>

#include "utils/AndroidUtils.h"

namespace carto {

    template <typename T>
    struct JNIUniqueGlobalRef {
        JNIUniqueGlobalRef() = default;
        JNIUniqueGlobalRef(const JNIUniqueGlobalRef&) = delete;
        JNIUniqueGlobalRef(JNIUniqueGlobalRef&& other) { std::swap(_ref, other._ref); }
        explicit JNIUniqueGlobalRef(jobject obj) : _ref(reinterpret_cast<T>(obj)) { }
        ~JNIUniqueGlobalRef() { if (_ref) { JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv(); if (jenv) jenv->DeleteGlobalRef(_ref); } }

        T get() const { return _ref; }
        operator T() const { return _ref; }
        JNIUniqueGlobalRef& operator = (const JNIUniqueGlobalRef&) = delete;
        JNIUniqueGlobalRef& operator = (JNIUniqueGlobalRef&& other) { std::swap(_ref, other._ref); return *this; }

    private:
        T _ref = NULL;
    };

}

#endif
