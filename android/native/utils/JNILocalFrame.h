/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_JNILOCALFRAME_H_
#define _CARTO_JNILOCALFRAME_H_

#include <jni.h>

#include "utils/Log.h"

namespace carto {

    struct JNILocalFrame {
        JNILocalFrame() = default;
        JNILocalFrame(const JNILocalFrame&) = delete;
        JNILocalFrame(JNILocalFrame&& other) { std::swap(_jenv, other._jenv); }
        JNILocalFrame(JNIEnv* jenv, int count, const char* methodId) : _jenv(jenv) {
            if (_jenv) {
                if (_jenv->PushLocalFrame(count) < 0) { 
                    Log::Errorf("%s: Failed to reserve local JNI frame!", methodId);
                    _jenv = NULL;
                }
            }
        }
        ~JNILocalFrame() { if (_jenv) { _jenv->PopLocalFrame(NULL); } }

        bool isValid() const { return _jenv != NULL; }
        JNILocalFrame& operator = (const JNILocalFrame&) = delete;
        JNILocalFrame& operator = (JNILocalFrame&& other) { std::swap(_jenv, other._jenv); return *this; }

    private:
        JNIEnv* _jenv = NULL;
    };

}

#endif
