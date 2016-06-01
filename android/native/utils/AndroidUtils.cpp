#include "AndroidUtils.h"
#include "utils/Log.h"

namespace carto {

    AndroidUtils::JNILocalFrame::JNILocalFrame(JNIEnv* jenv, int count, const char* methodId) :
        _jenv(jenv), _valid(false)
    {
        if (_jenv->PushLocalFrame(count) < 0) { 
            Log::Errorf("%s: Failed to reserve local JNI frame!", methodId);
            return;
        }
        _valid = true;
    }

    AndroidUtils::JNILocalFrame::~JNILocalFrame() {
        if (_valid) {
            _jenv->PopLocalFrame(NULL);
        }
    }

    bool AndroidUtils::JNILocalFrame::isValid() const {
        return _valid;
    }

    void AndroidUtils::AttachJVM(JNIEnv* jenv) {
        std::lock_guard<std::mutex> lock(_Mutex);
        if (_JVM) {
            return;
        }
        Log::Info("AndroidUtils::AttachJVM: Attaching JVM from current JNIEnv");

        int getVMStat = jenv->GetJavaVM(&_JVM);
        if (getVMStat != JNI_OK) {
            Log::Fatal("AndroidUtils::AttachJVM: Could not get JVM instance!");
            return;
        }

        pthread_key_create(&_JNIEnvKey, AndroidUtils::DetachCurrentThread);
    }

    void AndroidUtils::SetContext(jobject context) {
        JNIEnv* jenv = GetCurrentThreadJNIEnv();

        jenv->PushLocalFrame(32);

        jclass contextClass = jenv->FindClass("android/content/Context");
        jmethodID getPackageNameMethod = jenv->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
        jstring packageNameString = (jstring)jenv->CallObjectMethod(context, getPackageNameMethod);
        jmethodID getContentResolverMethod = jenv->GetMethodID(contextClass, "getContentResolver", "()Landroid/content/ContentResolver;");
        jobject contentResolver = jenv->CallObjectMethod(context, getContentResolverMethod);

        jclass secureClass = jenv->FindClass("android/provider/Settings$Secure");
        jfieldID androidIdField = jenv->GetStaticFieldID(secureClass, "ANDROID_ID", "Ljava/lang/String;");
        jstring androidId = (jstring)jenv->GetStaticObjectField(secureClass, androidIdField);
        jmethodID getStringMethod = jenv->GetStaticMethodID(secureClass, "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
        jstring deviceIdString = (jstring)jenv->CallStaticObjectMethod(secureClass, getStringMethod, contentResolver, androidId);

        jclass buildVersionClass = jenv->FindClass("android/os/Build$VERSION");
        jfieldID releaseField = jenv->GetStaticFieldID(buildVersionClass, "RELEASE", "Ljava/lang/String;");
        jstring deviceOSString = (jstring)jenv->GetStaticObjectField(buildVersionClass, releaseField);
        jclass buildClass = jenv->FindClass("android/os/Build");
        jfieldID modelField = jenv->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
        jstring deviceTypeString = (jstring)jenv->GetStaticObjectField(buildClass, modelField);

        const char* packageName = "";
        if (packageNameString) {
            packageName = jenv->GetStringUTFChars(packageNameString, NULL);
        }
        const char* deviceId = "";
        if (deviceIdString) {
            deviceId = jenv->GetStringUTFChars(deviceIdString, NULL);
        }
        const char* deviceOS = "";
        if (deviceOSString) {
            deviceOS = jenv->GetStringUTFChars(deviceOSString, NULL);
        }
        const char* deviceType = "";
        if (deviceTypeString) {
            deviceType = jenv->GetStringUTFChars(deviceTypeString, NULL);
        }
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            _PackageName = packageName;
            _DeviceId = deviceId;
            _DeviceOS = deviceOS;
            _DeviceType = deviceType;
        }
        if (packageNameString) {
            jenv->ReleaseStringUTFChars(packageNameString, packageName);
        }
        if (deviceIdString) {
            jenv->ReleaseStringUTFChars(deviceIdString, deviceId);
        }
        if (deviceOSString) {
            jenv->ReleaseStringUTFChars(deviceOSString, deviceOS);
        }
        if (deviceTypeString) {
            jenv->ReleaseStringUTFChars(deviceTypeString, deviceType);
        }

        jenv->PopLocalFrame(NULL);
    }

    JavaVM* AndroidUtils::GetJVM() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _JVM;
    }

    JNIEnv* AndroidUtils::GetCurrentThreadJNIEnv() {
        std::lock_guard<std::mutex> lock(_Mutex);
        if (!_JVM) {
            Log::Error("AndroidUtils::GetCurrentThreadJNIEnv: JVM not attached!");
            return nullptr;
        }

        JNIEnv* jenv = nullptr;
        int getEnvStat = _JVM->GetEnv(reinterpret_cast<void**>(&jenv), JNI_VERSION_1_2);
        if (getEnvStat == JNI_OK) {
            return jenv;
        }
        if (getEnvStat != JNI_EDETACHED) {
            Log::Errorf("AndroidUtils::GetCurrentThreadJNIEnv: Failed with code %d!", getEnvStat);
            return jenv;
        }

        jenv = static_cast<JNIEnv*>(pthread_getspecific(_JNIEnvKey));
        if (!jenv) {
            jenv = AttachCurrentThread();
            pthread_setspecific(_JNIEnvKey, jenv);
        }
        return jenv;
    }

    std::string AndroidUtils::GetPackageName() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _PackageName;
    }

    std::string AndroidUtils::GetDeviceId() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _DeviceId;
    }

    std::string AndroidUtils::GetDeviceOS() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _DeviceOS;
    }

    std::string AndroidUtils::GetDeviceType() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _DeviceType;
    }

    AndroidUtils::AndroidUtils() {
    }

    JNIEnv* AndroidUtils::AttachCurrentThread() {
        Log::Info("AndroidUtils::AttachCurrentThread: Attaching current thread to JNI");

        JNIEnv* jenv = nullptr;
        int attachStat = _JVM->AttachCurrentThread(&jenv, NULL);
        if (attachStat != JNI_OK) {
            Log::Error("AndroidUtils::AttachCurrentThread: Failed to attach current thread to JNI!");
            return nullptr;
        }
        return jenv;
    }

    void AndroidUtils::DetachCurrentThread(void* jenv) {
        if (jenv) {
            Log::Info("AndroidUtils::DetachCurrentThread: Detaching current thread from JNI");
            _JVM->DetachCurrentThread();
        }
    }

    JavaVM* AndroidUtils::_JVM = nullptr;

    pthread_key_t AndroidUtils::_JNIEnvKey;

    std::string AndroidUtils::_PackageName;

    std::string AndroidUtils::_DeviceId;

    std::string AndroidUtils::_DeviceType;

    std::string AndroidUtils::_DeviceOS;

    std::mutex AndroidUtils::_Mutex;
}
