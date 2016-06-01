#include "utils/ThreadUtils.h"
#include "components/ThreadWorker.h"
#include "utils/Log.h"

#include <sys/resource.h>
#include <unistd.h>

namespace carto {

    void ThreadUtils::SetThreadPriority(ThreadPriority::ThreadPriority priority) {
        // setpriority is supposed to set the priority of the the whole process,
        // but most implementations set the priority of the specific thread instead, so it's ok
        errno = 0;
        int hasError = setpriority(PRIO_PROCESS, gettid(), priority);
        if (hasError != 0) {
            Log::Errorf("ThreadUtils::SetThreadPriority: Failed to set thread priority: %d, error: %s", priority, strerror(errno));
        }
    }

    ThreadUtils::ThreadUtils() {
    }

}
