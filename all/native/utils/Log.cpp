#include "utils/Log.h"

#ifdef __ANDROID__
#include <android/log.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <asl.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

namespace carto {

#ifdef __ANDROID__
    enum LogType { LOG_TYPE_FATAL = ANDROID_LOG_ERROR, LOG_TYPE_ERROR = ANDROID_LOG_ERROR, LOG_TYPE_WARNING = ANDROID_LOG_WARN, LOG_TYPE_INFO = ANDROID_LOG_INFO, LOG_TYPE_DEBUG = ANDROID_LOG_DEBUG };

    static void OutputLog(LogType logType, const std::string& tag, const char* text) {
        __android_log_print(static_cast<int>(logType), tag.c_str(), "%s", text);
    }
#endif
#ifdef __APPLE__
    enum LogType { LOG_TYPE_FATAL = ASL_LEVEL_EMERG, LOG_TYPE_ERROR = ASL_LEVEL_ERR, LOG_TYPE_WARNING = ASL_LEVEL_WARNING, LOG_TYPE_INFO = ASL_LEVEL_INFO, LOG_TYPE_DEBUG = ASL_LEVEL_DEBUG };

    static void OutputLog(LogType logType, const std::string& tag, const char* text) {
        asl_log(NULL, NULL, static_cast<int>(logType), "%s", text);
    }
#endif
#ifdef _WIN32
    enum LogType { LOG_TYPE_FATAL, LOG_TYPE_ERROR, LOG_TYPE_WARNING, LOG_TYPE_INFO, LOG_TYPE_DEBUG };
    
    static void OutputLog(LogType logType, const std::string& tag, const char* text) {
        OutputDebugStringA(text);
    }
#endif

    bool Log::IsShowError() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _ShowError;
    }

    void Log::SetShowError(bool showError) {
        std::lock_guard<std::mutex> lock(_Mutex);
        _ShowError = showError;
    }

    bool Log::IsShowWarn() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _ShowWarn;
    }

    void Log::SetShowWarn(bool showWarn) {
        std::lock_guard<std::mutex> lock(_Mutex);
        _ShowWarn = showWarn;
    }

    bool Log::IsShowInfo() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _ShowInfo;
    }

    void Log::SetShowInfo(bool showInfo) {
        std::lock_guard<std::mutex> lock(_Mutex);
        _ShowInfo = showInfo;
    }

    bool Log::IsShowDebug() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _ShowDebug;
    }

    void Log::SetShowDebug(bool showDebug) {
        std::lock_guard<std::mutex> lock(_Mutex);
        _ShowDebug = showDebug;
    }

    std::string Log::GetTag() {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _Tag;
    }

    void Log::SetTag(const std::string& tag) {
        std::lock_guard<std::mutex> lock(_Mutex);
        _Tag = tag;
    }

    void Log::Fatal(const char* text) {
        std::lock_guard<std::mutex> lock(_Mutex);
        OutputLog(LOG_TYPE_FATAL, _Tag, text);
    }

    void Log::Error(const char* text) {
        std::lock_guard<std::mutex> lock(_Mutex);
        OutputLog(LOG_TYPE_ERROR, _Tag, text);
    }

    void Log::Warn(const char* text) {
        std::lock_guard<std::mutex> lock(_Mutex);
        OutputLog(LOG_TYPE_WARNING, _Tag, text);
    }

    void Log::Info(const char* text) {
        std::lock_guard<std::mutex> lock(_Mutex);
        OutputLog(LOG_TYPE_INFO, _Tag, text);
    }

    void Log::Debug(const char* text) {
        std::lock_guard<std::mutex> lock(_Mutex);
        OutputLog(LOG_TYPE_DEBUG, _Tag, text);
    }

    Log::Log() {
    }

    bool Log::_ShowError = true;
    bool Log::_ShowWarn = true;
    bool Log::_ShowInfo = true;
    bool Log::_ShowDebug = true;

    std::string Log::_Tag = "carto-mobile-sdk";

    std::mutex Log::_Mutex;

}
