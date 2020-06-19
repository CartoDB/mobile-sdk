/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_THREADUTILS_H
#define _CARTO_THREADUTILS_H

#include <thread>

namespace carto {

    namespace ThreadPriority {
        enum ThreadPriority {
            MINIMUM = 20,
            LOW     = 10,
            NORMAL  = 0,
            HIGH    = -10,
            MAXIMUM = -20
        };
    }

    class ThreadUtils {
    public:
        static void SetThreadPriority(ThreadPriority::ThreadPriority priority);

    private:
        ThreadUtils();
    };

}

#endif
