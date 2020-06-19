#include "utils/ThreadUtils.h"
#include "components/ThreadWorker.h"
#include "utils/Log.h"

#include <sys/resource.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

namespace carto {

    void ThreadUtils::SetThreadPriority(ThreadPriority::ThreadPriority priority) {
        // setpriority is supposed to set the priority of the the whole process, but by using thread id, it is possible to set the priority of actual thread on Linux/Android.
        int posixPriority = std::min(19, (static_cast<int>(priority) - static_cast<int>(ThreadPriority::MAXIMUM)) * 40 / (static_cast<int>(ThreadPriority::MINIMUM) - static_cast<int>(ThreadPriority::MAXIMUM)) - 20);
        int hasError = ::setpriority(PRIO_PROCESS, gettid(), posixPriority);
        if (hasError != 0) {
            Log::Errorf("ThreadUtils::SetThreadPriority: Failed to set thread priority: %d", posixPriority);
        }
    }

    ThreadUtils::ThreadUtils() {
    }

}
