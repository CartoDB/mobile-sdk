/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_JNIUNIQUELOCALREF_H_
#define _CARTO_JNIUNIQUELOCALREF_H_

#include <jni.h>

#include "utils/AndroidUtils.h"

namespace carto {

    template <typename T>
    struct JNIUniqueLocalRef {
        JNIUniqueLocalRef() = default;
        JNIUniqueLocalRef(const JNIUniqueLocalRef&) = delete;
        JNIUniqueLocalRef(JNIUniqueLocalRef&& other) { std::swap(_ref, other._ref); }
        explicit JNIUniqueLocalRef(jobject obj) : _ref(reinterpret_cast<T>(obj)) { }
        ~JNIUniqueLocalRef() { if (_ref) { JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv(); if (jenv) jenv->DeleteLocalRef(_ref); } }

        T get() const { return _ref; }
        operator T() const { return _ref; }
        JNIUniqueLocalRef& operator = (const JNIUniqueLocalRef&) = delete;
        JNIUniqueLocalRef& operator = (JNIUniqueLocalRef&& other) { std::swap(_ref, other._ref); return *this; }

    private:
        T _ref = NULL;
    };

}

#endif
