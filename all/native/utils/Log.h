/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LOG_H_
#define _CARTO_LOG_H_

#include <mutex>
#include <string>

namespace carto {

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
         * Logs specified fatal error message and terminates.
         * @param text The text to log.
         */
        static void Fatal(const char* text);
        /**
         * Logs specified error message (if error logging is enabled).
         * @param text The text to log.
         */
        static void Error(const char* text);
        /**
         * Logs specified warning message (if warning logging is enabled).
         * @param text The text to log.
         */
        static void Warn(const char* text);
        /**
         * Logs specified info message (if info logging is enabled).
         * @param text The text to log.
         */
        static void Info(const char* text);
        /**
         * Logs specified debug message (if debug logging is enabled).
         * @param text The text to log.
         */
        static void Debug(const char* text);

        static void Fatalf(const char* text, ...);
        static void Errorf(const char* text, ...);
        static void Warnf(const char* text, ...);
        static void Infof(const char* text, ...);
        static void Debugf(const char* text, ...);

    private:
        Log();

        static bool _ShowError;
        static bool _ShowWarn;
        static bool _ShowInfo;
        static bool _ShowDebug;

        static std::string _Tag;

        static std::mutex _Mutex;
    };

}

#endif
