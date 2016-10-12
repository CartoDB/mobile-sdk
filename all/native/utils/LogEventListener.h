/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LOGEVENTLISTENER_H_
#define _CARTO_LOGEVENTLISTENER_H_

#include <string>

namespace carto {
    /**
     * Listener for events logged by the SDK.
     */
    class LogEventListener {
    public:
        virtual ~LogEventListener() { }
    
        /**
         * Listener method that gets called when SDK want to log a debug level event.
         * This method can be called from any thread.
         * @param message The message to log.
         * @return True if the message should be shown by the SDK or false if it should be ignored.
         */
        virtual bool onDebugEvent(const std::string& message) { return true; }

        /**
         * Listener method that gets called when SDK want to log an info level event.
         * This method can be called from any thread.
         * @param message The message to log.
         * @return True if the message should be shown by the SDK or false if it should be ignored.
         */
        virtual bool onInfoEvent(const std::string& message) { return true; }

        /**
         * Listener method that gets called when SDK want to log a warning level event.
         * This method can be called from any thread.
         * @param message The message to log.
         * @return True if the message should be shown by the SDK or false if it should be ignored.
         */
        virtual bool onWarnEvent(const std::string& message) { return true; }

        /**
         * Listener method that gets called when SDK want to log an error level event.
         * This method can be called from any thread.
         * @param message The message to log.
         * @return True if the message should be shown by the SDK or false if it should be ignored.
         */
        virtual bool onErrorEvent(const std::string& message) { return true; }

        /**
         * Listener method that gets called when SDK want to log a fatal level event.
         * This method can be called from any thread.
         * @param message The message to log.
         * @return True if the message should be shown by the SDK or false if it should be ignored.
         */
        virtual bool onFatalEvent(const std::string& message) { return true; }
    };
    
}

#endif
