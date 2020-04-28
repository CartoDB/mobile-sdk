/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_JNILOCALFRAME_H_
#define _CARTO_JNILOCALFRAME_H_

#include <jni.h>

#include "utils/AndroidUtils.h"
#include "utils/Log.h"

namespace carto {

    struct JNILocalFrame {
        JNILocalFrame(JNIEnv* jenv, int count, const char* methodId) : _jenv(jenv ? jenv : AndroidUtils::GetCurrentThreadJNIEnv()) {
            if (!(_jenv && _jenv->PushLocalFrame(count) >= 0)) { 
                Log::Errorf("%s: Failed to reserve local JNI frame!", methodId);
                _jenv = NULL;
            }
        }
        JNILocalFrame(int count, const char* methodId) : _jenv(AndroidUtils::GetCurrentThreadJNIEnv()) {
            if (!(_jenv && _jenv->PushLocalFrame(count) >= 0)) { 
                Log::Errorf("%s: Failed to reserve local JNI frame!", methodId);
                _jenv = NULL;
            }
        }
        ~JNILocalFrame() { if (_jenv) { _jenv->PopLocalFrame(NULL); } }

        bool isValid() const { return _jenv != NULL; }

    private:
        JNIEnv* _jenv = NULL;
    };

}

#endif
