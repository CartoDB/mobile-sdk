#include "HTTPClientAndroidImpl.h"
#include "components/Exceptions.h"
#include "utils/AndroidUtils.h"
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
            clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("java/net/URL")));
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
        jmethodID connect;
        jmethodID disconnect;
        jmethodID getResponseCode;
        jmethodID getHeaderFieldKey;
        jmethodID getHeaderField;
        jmethodID getInputStream;
        jmethodID getOutputStream;
        jmethodID getErrorStream;
        
        explicit HttpURLConnectionClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("java/net/HttpURLConnection")));
            setRequestMethod = jenv->GetMethodID(clazz, "setRequestMethod", "(Ljava/lang/String;)V");
            setDoInput = jenv->GetMethodID(clazz, "setDoInput", "(Z)V");
            setDoOutput = jenv->GetMethodID(clazz, "setDoOutput", "(Z)V");
            setUseCaches = jenv->GetMethodID(clazz, "setUseCaches", "(Z)V");
            setAllowUserInteraction = jenv->GetMethodID(clazz, "setAllowUserInteraction", "(Z)V");
            setInstanceFollowRedirects = jenv->GetMethodID(clazz, "setInstanceFollowRedirects", "(Z)V");
            setRequestProperty = jenv->GetMethodID(clazz, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
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
            clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("java/io/InputStream")));
            read = jenv->GetMethodID(clazz, "read", "([B)I");
            close = jenv->GetMethodID(clazz, "close", "()V");
        }
    };
    
    struct HTTPClient::AndroidImpl::OutputStreamClass {
        JNIUniqueGlobalRef<jclass> clazz;
        jmethodID write;
        jmethodID close;

        explicit OutputStreamClass(JNIEnv* jenv) {
            clazz = JNIUniqueGlobalRef<jclass>(jenv->NewGlobalRef(jenv->FindClass("java/io/OutputStream")));
            write = jenv->GetMethodID(clazz, "write", "([B)V");
            close = jenv->GetMethodID(clazz, "close", "()V");
        }
    };
    
    HTTPClient::AndroidImpl::AndroidImpl(bool log) :
        _log(log)
    {
    }
    
    bool HTTPClient::AndroidImpl::makeRequest(const HTTPClient::Request& request, HeadersFn headersFn, DataFn dataFn) const {
        JNIEnv* jenv = AndroidUtils::GetCurrentThreadJNIEnv();
        AndroidUtils::JNILocalFrame jframe(jenv, 32, "HTTPClient::AndroidImpl::HTTPClientAndroidImpl");
        if (!jframe.isValid()) {
            Log::Error("HTTPClient::AndroidImpl::makeRequest: JNILocalFrame not valid");
            throw std::runtime_error("JNILocalFrame not valid");
        }
        
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            if (!_URLClass) {
                _URLClass = std::unique_ptr<URLClass>(new URLClass(jenv));
            }
            if (!_HttpURLConnectionClass) {
                _HttpURLConnectionClass = std::unique_ptr<HttpURLConnectionClass>(new HttpURLConnectionClass(jenv));
            }
            if (!_InputStreamClass) {
                _InputStreamClass = std::unique_ptr<InputStreamClass>(new InputStreamClass(jenv));
            }
            if (!_OutputStreamClass) {
                _OutputStreamClass = std::unique_ptr<OutputStreamClass>(new OutputStreamClass(jenv));
            }
        }

        // Create URL
        jobject url = jenv->NewObject(_URLClass->clazz, _URLClass->constructor, jenv->NewStringUTF(request.url.c_str()));
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw NetworkException("Invalid URL", request.url);
        }
        
        // Open HTTP connection
        jobject conn = jenv->CallObjectMethod(url, _URLClass->openConnection);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw NetworkException("Unable to open connection", request.url);
        }
        
        // Configure connection parameters
        jenv->CallVoidMethod(conn, _HttpURLConnectionClass->setRequestMethod, jenv->NewStringUTF("GET"));
        jenv->CallVoidMethod(conn, _HttpURLConnectionClass->setDoInput, (jboolean)true);
        jenv->CallVoidMethod(conn, _HttpURLConnectionClass->setDoOutput, (jboolean)!request.contentType.empty());
        jenv->CallVoidMethod(conn, _HttpURLConnectionClass->setUseCaches, (jboolean)false);
        jenv->CallVoidMethod(conn, _HttpURLConnectionClass->setAllowUserInteraction, (jboolean)false);
        jenv->CallVoidMethod(conn, _HttpURLConnectionClass->setAllowUserInteraction, (jboolean)true);

        // Set request headers
        for (auto it = request.headers.begin(); it != request.headers.end(); it++) {
            jstring key = jenv->NewStringUTF(it->first.c_str());
            jstring value = jenv->NewStringUTF(it->second.c_str());
            jenv->CallVoidMethod(conn, _HttpURLConnectionClass->setRequestProperty, key, value);
        }

        // If Content-Type is set, write request body to output stream
        if (!request.contentType.empty()) {
            jobject outputStream = jenv->CallObjectMethod(conn, _HttpURLConnectionClass->getOutputStream);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw NetworkException("Unable to get output stream", request.url);
            }

            jbyteArray jbuf = jenv->NewByteArray(request.body.size());
            jenv->SetByteArrayRegion(jbuf, 0, request.body.size(), reinterpret_cast<const jbyte*>(request.body.data()));

            jenv->CallVoidMethod(outputStream, _OutputStreamClass->write, jbuf);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw NetworkException("Unable to write data", request.url);
            }
            jenv->CallVoidMethod(outputStream, _OutputStreamClass->close);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
                throw NetworkException("Unable to write data", request.url);
            }
        }

        // Connect
        jenv->CallVoidMethod(conn, _HttpURLConnectionClass->connect);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw NetworkException("Unable to connect", request.url);
        }
        
        // Read response header
        jint responseCode = jenv->CallIntMethod(conn, _HttpURLConnectionClass->getResponseCode);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            throw NetworkException("Unable to read response code", request.url);
        }
        std::map<std::string, std::string> headers;
        for (int i = 0; true; i++) {
            jstring key = (jstring)jenv->CallObjectMethod(conn, _HttpURLConnectionClass->getHeaderFieldKey, (jint)i);
            if (!key) {
                break;
            }
            jstring value = (jstring)jenv->CallObjectMethod(conn, _HttpURLConnectionClass->getHeaderField, (jint)i);
            const char* keyStr = jenv->GetStringUTFChars(key, NULL);
            const char* valueStr = jenv->GetStringUTFChars(value, NULL);
            headers[keyStr] = valueStr;
            jenv->ReleaseStringUTFChars(value, valueStr);
            jenv->ReleaseStringUTFChars(key, keyStr);
        }

        bool cancel = false;
        if (!headersFn(responseCode, headers)) {
            cancel = true;
        }

        // Get input stream
        jobject inputStream = jenv->CallObjectMethod(conn, _HttpURLConnectionClass->getInputStream);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
            inputStream = jenv->CallObjectMethod(conn, _HttpURLConnectionClass->getErrorStream);
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
                jint numBytesRead = jenv->CallIntMethod(inputStream, _InputStreamClass->read, jbuf);

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
            jenv->CallVoidMethod(inputStream, _InputStreamClass->close);
            if (jenv->ExceptionCheck()) {
                jenv->ExceptionClear();
            }
            jenv->CallVoidMethod(conn, _HttpURLConnectionClass->disconnect);
            throw;
        }
        
        // Done
        jenv->CallVoidMethod(inputStream, _InputStreamClass->close);
        if (jenv->ExceptionCheck()) {
            jenv->ExceptionClear();
        }

        // If cancelled, explicitly disconnect
        if (cancel) {
            jenv->CallVoidMethod(conn, _HttpURLConnectionClass->disconnect);
            return false;
        }

        return true;
    }
    
    std::unique_ptr<HTTPClient::AndroidImpl::URLClass> HTTPClient::AndroidImpl::_URLClass;
    std::unique_ptr<HTTPClient::AndroidImpl::HttpURLConnectionClass> HTTPClient::AndroidImpl::_HttpURLConnectionClass;
    std::unique_ptr<HTTPClient::AndroidImpl::InputStreamClass> HTTPClient::AndroidImpl::_InputStreamClass;
    std::unique_ptr<HTTPClient::AndroidImpl::OutputStreamClass> HTTPClient::AndroidImpl::_OutputStreamClass;
    
    std::mutex HTTPClient::AndroidImpl::_Mutex;
    
}
