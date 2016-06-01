/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ANDROIDUTILS_H_
#define _CARTO_ANDROIDUTILS_H_

#include <mutex>
#include <string>

#include <jni.h>
#include <pthread.h>

namespace carto {

    /**
     * AndroidUtils is an internal class of the SDK.
     * It is not intended for public usage.
     */
    class AndroidUtils {
    public:
        struct JNILocalFrame {
            JNILocalFrame(JNIEnv* jenv, int count, const char* methodId);
            ~JNILocalFrame();

            bool isValid() const;

        private:
            JNIEnv* _jenv;
            bool _valid;
        };

        static void AttachJVM(JNIEnv* jenv);
    static void SetContext(jobject context);

        static JavaVM* GetJVM();
        static JNIEnv* GetCurrentThreadJNIEnv();

        static std::string GetPackageName();
        static std::string GetDeviceId();
        static std::string GetDeviceType();
        static std::string GetDeviceOS();

    private:
        AndroidUtils();

        static JNIEnv* AttachCurrentThread();
        static void DetachCurrentThread(void* jenv);

        static JavaVM* _JVM;
        static pthread_key_t _JNIEnvKey;

        static std::string _PackageName;
        static std::string _DeviceId;
        static std::string _DeviceType;
        static std::string _DeviceOS;

        static std::mutex _Mutex;
    };

}

#endif
