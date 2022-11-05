#include "HTTPClientAndroidImpl.h"
#include "components/Exceptions.h"
#include "utils/AndroidUtils.h"
#include "utils/JNILocalFrame.h"
#include "utils/JNIUniqueLocalRef.h"
#include "utils/JNIUniqueGlobalRef.h"
#include "utils/Log.h"

#include <chrono>
#include <limits>
#include <regex>

#include <boost/lexical_cast.hpp>

namespace carto {
    
    struct HTTPClient::AndroidImpl::URLClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID constructor;
        jmethodID openConnection;
        
        explicit URLClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("java/net/URL")));
            constructor = jenv->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
            openConnection = jenv->GetMethodID(clazz, "openConnection", "()Ljava/net/URLConnection;");
        }
    };
    
    struct HTTPClient::AndroidImpl::HttpURLConnectionClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID setRequestMethod;
        jmethodID setDoInput;
        jmethodID setDoOutput;
        jmethodID setUseCaches;
        jmethodID setAllowUserInteraction;
        jmethodID setInstanceFollowRedirects;
        jmethodID setRequestProperty;
        jmethodID setConnectTimeout;
        jmethodID setReadTimeout;
        jmethodID connect;
        jmethodID disconnect;
        jmethodID getResponseCode;
        jmethodID getHeaderFieldKey;
        jmethodID getHeaderField;
        jmethodID getInputStream;
        jmethodID getOutputStream;
        jmethodID getErrorStream;
        
        explicit HttpURLConnectionClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("java/net/HttpURLConnection")));
            setRequestMethod = jenv->GetMethodID(clazz, "setRequestMethod", "(Ljava/lang/String;)V");
            setDoInput = jenv->GetMethodID(clazz, "setDoInput", "(Z)V");
            setDoOutput = jenv->GetMethodID(clazz, "setDoOutput", "(Z)V");
            setUseCaches = jenv->GetMethodID(clazz, "setUseCaches", "(Z)V");
            setAllowUserInteraction = jenv->GetMethodID(clazz, "setAllowUserInteraction", "(Z)V");
            setInstanceFollowRedirects = jenv->GetMethodID(clazz, "setInstanceFollowRedirects", "(Z)V");
            setRequestProperty = jenv->GetMethodID(clazz, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
            setConnectTimeout = jenv->GetMethodID(clazz, "setConnectTimeout", "(I)V");
            setReadTimeout = jenv->GetMethodID(clazz, "setReadTimeout", "(I)V");
            connect = jenv->GetMethodID(clazz, "connect", "()V");
            disconnect = jenv->GetMethodID(clazz, "disconnect", "()V");
            getResponseCode = jenv->GetMethodID(clazz, "getResponseCode", "()I");
            getHeaderFieldKey = jenv->GetMethodID(clazz, "getHeaderFieldKey", "(I)Ljava/lang/String;");
            getHeaderField = jenv->GetMethodID(clazz, "getHeaderField", "(I)Ljava/lang/String;");
            getInputStream = jenv->GetMethodID(clazz, "getInputStream", "()Ljava/io/InputStream;");
            getOutputStream = jenv->GetMethodID(clazz, "getOutputStream", "()Ljava/io/OutputStream;");
            getErrorStream = jenv->GetMethodID(clazz, "getErrorStream", "()Ljava/io/InputStream;");
        }
    };
    
    struct HTTPClient::AndroidImpl::InputStreamClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID read;
        jmethodID close;

        explicit InputStreamClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("java/io/InputStream")));
            read = jenv->GetMethodID(clazz, "read", "([B)I");
            close = jenv->GetMethodID(clazz, "close", "()V");
        }
    };
    
    struct HTTPClient::AndroidImpl::OutputStreamClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID write;
        jmethodID close;

        explicit OutputStreamClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv, jenv->NewGlobalRef(jenv->FindClass("java/io/OutputStream")));
            write = jenv->GetMethodID(clazz, "write", "([B)V");
            close = jenv->GetMethodID(clazz, "close", "()V");
        }
    };
    
    HTTPClient::AndroidImpl::AndroidImpl(bool log) :
        _log(log),
        _timeout(-1)
    {
    }

    void HTTPClient::AndroidImpl::setTimeout(int milliseconds) {
        _timeout = milliseconds;
    }
    
    bool HTTPClient::AndroidImpl::makeRequest(const HTTPClient::Request& request, HeadersFunc headersFn, DataFunc dataFn) const {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        JNILocalFrame jframe(jenv, 32, "HTTPClient::AndroidImpl::HTTPClientAndroidImpl");
        if (!jframe.isValid()) {
            Log::Error("HTTPClient::AndroidImpl::makeRequest: JNILocalFrame not valid");
            return false;
        }
        
        // Create URL
        jobject url = jenv->NewObject(GetURLClass()->clazz, GetURLClass()->constructor, jenv->NewStringUTF(request.url.c_str()));
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw NetworkException("Invalid URL", request.url);
        }

        // Open HTTP connection
        jobject conn = jenv->CallObjectMethod(url, GetURLClass()->openConnection);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw NetworkException("Unable to open connection", request.url);
        }
        
        // Configure connection parameters
        jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setRequestMethod, jenv->NewStringUTF(request.method.c_str()));
        jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setDoInput, (jboolean)true);
        jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setDoOutput, (jboolean)!request.contentType.empty());
        jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setUseCaches, (jboolean)false);
        jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setAllowUserInteraction, (jboolean)false);
        jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setInstanceFollowRedirects, (jboolean)true);
        int timeout = _timeout.load();
        if (timeout > 0) {
            jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setConnectTimeout, timeout);
            jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setReadTimeout, timeout);
        }
        
        // Set request headers
        for (auto it = request.headers.begin(); it != request.headers.end(); it++) {
            JNIUniqueLocalRef<jstring> key(jenv, jenv->NewStringUTF(it->first.c_str()));
            JNIUniqueLocalRef<jstring> value(jenv, jenv->NewStringUTF(it->second.c_str()));
            jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->setRequestProperty, key.get(), value.get());
        }

        // If Content-Type is set, write request body to output stream
        if (!request.contentType.empty()) {
            jobject outputStream = jenv->CallObjectMethod(conn, GetHttpURLConnectionClass()->getOutputStream);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw NetworkException("Unable to get output stream", request.url);
            }

            jbyteArray jbuf = jenv->NewByteArray(request.body.size());
            jenv->SetByteArrayRegion(jbuf, 0, request.body.size(), reinterpret_cast<const jbyte*>(request.body.data()));

            jenv->CallVoidMethod(outputStream, GetOutputStreamClass()->write, jbuf);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw NetworkException("Unable to write data", request.url);
            }
            jenv->CallVoidMethod(outputStream, GetOutputStreamClass()->close);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw NetworkException("Unable to write data", request.url);
            }
        }

        // Connect
        jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->connect);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw NetworkException("Unable to connect", request.url);
        }
        
        // Read response header
        jint responseCode = jenv->CallIntMethod(conn, GetHttpURLConnectionClass()->getResponseCode);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw NetworkException("Unable to read response code", request.url);
        }
        std::map<std::string, std::string> headers;
        for (int i = 0; true; i++) {
            JNIUniqueLocalRef<jstring> key(jenv, jenv->CallObjectMethod(conn, GetHttpURLConnectionClass()->getHeaderFieldKey, (jint)i));
            if (!key) {
                break;
            }
            JNIUniqueLocalRef<jstring> value(jenv, jenv->CallObjectMethod(conn, GetHttpURLConnectionClass()->getHeaderField, (jint)i));

            const char* keyStr = jenv->GetStringUTFChars(key.get(), NULL);
            const char* valueStr = jenv->GetStringUTFChars(value.get(), NULL);
            headers[keyStr] = valueStr;
            jenv->ReleaseStringUTFChars(value.get(), valueStr);
            jenv->ReleaseStringUTFChars(key.get(), keyStr);
        }

        bool cancel = false;
        if (!headersFn(responseCode, headers)) {
            cancel = true;
        }

        // Get input stream
        jobject inputStream = jenv->CallObjectMethod(conn, GetHttpURLConnectionClass()->getInputStream);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            inputStream = jenv->CallObjectMethod(conn, GetHttpURLConnectionClass()->getErrorStream);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw NetworkException("Unable to get input stream", request.url);
            }
        }
        
        try {
            jbyte buf[4096];
            jbyteArray jbuf = jenv->NewByteArray(sizeof(buf));

            std::uint64_t readOffset = 0;
            while (!cancel) {
                jint numBytesRead = jenv->CallIntMethod(inputStream, GetInputStreamClass()->read, jbuf);

                if (jenv->ExceptionCheck()) {
                    jenv->ExceptionClear();
                    throw NetworkException("Unable to read data", request.url);
                }
                if (numBytesRead < 0) {
                    break;
                }
                jenv->GetByteArrayRegion(jbuf, 0, numBytesRead, buf);
            
                if (!dataFn(reinterpret_cast<const unsigned char*>(&buf[0]), numBytesRead)) {
                    cancel = true;
                }

                readOffset += numBytesRead;
            }
        }
        catch (...) {
            jenv->CallVoidMethod(inputStream, GetInputStreamClass()->close);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
            }
            jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->disconnect);
            throw;
        }
        
        // Done
        jenv->CallVoidMethod(inputStream, GetInputStreamClass()->close);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
        }

        // If cancelled, explicitly disconnect
        if (cancel) {
            jenv->CallVoidMethod(conn, GetHttpURLConnectionClass()->disconnect);
            return false;
        }

        return true;
    }
    
    std::unique_ptr<HTTPClient::AndroidImpl::URLClass>& HTTPClient::AndroidImpl::GetURLClass() {
        static std::unique_ptr<URLClass> cls(new URLClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<HTTPClient::AndroidImpl::HttpURLConnectionClass>& HTTPClient::AndroidImpl::GetHttpURLConnectionClass() {
        static std::unique_ptr<HttpURLConnectionClass> cls(new HttpURLConnectionClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<HTTPClient::AndroidImpl::InputStreamClass>& HTTPClient::AndroidImpl::GetInputStreamClass() {
        static std::unique_ptr<InputStreamClass> cls(new InputStreamClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }

    std::unique_ptr<HTTPClient::AndroidImpl::OutputStreamClass>& HTTPClient::AndroidImpl::GetOutputStreamClass() {
        static std::unique_ptr<OutputStreamClass> cls(new OutputStreamClass(AndroidUtils::GetCurrentThreadJNIEnv()));
        return cls;
    }
    
}
