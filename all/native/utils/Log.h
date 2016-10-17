/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LOG_H_
#define _CARTO_LOG_H_

#include "components/DirectorPtr.h"

#include <mutex>
#include <string>
#include <memory>

#include <tinyformat.h>

namespace carto {
    class LogEventListener;

    /**
     * A diagnostic log for various SDK events.
     */
    class Log {
    public:
        /**
         * Returns the state of error logging.
         * @return True if errors are shown in the log.
         */
        static bool IsShowError();
        /**
         * Enables or disables writing error messages to the log.
         * @param showError If true, then error messages will be written to the log.
         */
        static void SetShowError(bool showError);
        /**
         * Returns the state of warning logging.
         * @return True if warnings are shown in the log.
         */
        static bool IsShowWarn();
        /**
         * Enables or disables writing warning messages to the log.
         * @param showWarn If true, then warning messages will be written to the log.
         */
        static void SetShowWarn(bool showWarn);
        /**
         * Returns the state of general info logging.
         * @return True if general info is shown in the log.
         */
        static bool IsShowInfo();
        /**
         * Enables or disables writing info messages to the log.
         * @param showInfo If true, then info messages will be written to the log.
         */
        static void SetShowInfo(bool showInfo);
        /**
         * Returns the state of internal debug message logging.
         * @return True if debug messages are shown in the log.
         */
        static bool IsShowDebug();
        /**
         * Enables or disables writing internal debug messages to the log.
         * @param showDebug If true, then debug messages will be written to the log.
         */
        static void SetShowDebug(bool showDebug);

        /**
         * Returns the tag for the log events.
         * @return The current tag for the log events.
         */
        static std::string GetTag();
        /**
         * Sets the tag for the log events. The tag will be visible in the log and log messages can be filtered by the tag.
         * @param tag The tag to use in the log events.
         */
        static void SetTag(const std::string& tag);

        /**
         * Returns the current log listener.
         * @return The current log event listener.
         */
        static std::shared_ptr<LogEventListener> GetLogEventListener();
        /**
         * Sets the log listener that can be used to intercept log messages.
         * @param listener The log event listener.
         */
        static void SetLogEventListener(const std::shared_ptr<LogEventListener>& listener);

        /**
         * Logs specified fatal error message and terminates.
         * @param message The message to log.
         */
        static void Fatal(const char* message);
        /**
         * Logs specified error message (if error logging is enabled).
         * @param message The message to log.
         */
        static void Error(const char* message);
        /**
         * Logs specified warning message (if warning logging is enabled).
         * @param message The message to log.
         */
        static void Warn(const char* message);
        /**
         * Logs specified info message (if info logging is enabled).
         * @param message The message to log.
         */
        static void Info(const char* message);
        /**
         * Logs specified debug message (if debug logging is enabled).
         * @param message The message to log.
         */
        static void Debug(const char* message);

#ifndef SWIG
        template <typename... Args>
        static void Fatalf(const char* formatString, const Args&... args) {
            std::string msg = tfm::format(formatString, args...);
            Fatal(msg.c_str());
        }

        template <typename... Args>
        static void Errorf(const char* formatString, const Args&... args) {
            std::string msg = tfm::format(formatString, args...);
            Error(msg.c_str());
        }

        template <typename... Args>
        static void Warnf(const char* formatString, const Args&... args) {
            std::string msg = tfm::format(formatString, args...);
            Warn(msg.c_str());
        }

        template <typename... Args>
        static void Infof(const char* formatString, const Args&... args) {
            std::string msg = tfm::format(formatString, args...);
            Info(msg.c_str());
        }

        template <typename... Args>
        static void Debugf(const char* formatString, const Args&... args) {
            std::string msg = tfm::format(formatString, args...);
            Debug(msg.c_str());
        }
#endif

    private:
        Log();

        static bool _ShowError;
        static bool _ShowWarn;
        static bool _ShowInfo;
        static bool _ShowDebug;

        static std::string _Tag;

        static DirectorPtr<LogEventListener> _LogEventListener;

        static std::mutex _Mutex;
    };

}

#endif
