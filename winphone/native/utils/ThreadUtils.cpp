#include "utils/ThreadUtils.h"
#include "components/ThreadWorker.h"
#include "utils/Log.h"

#include <windows.h>

namespace carto {

    void ThreadUtils::SetThreadPriority(ThreadPriority::ThreadPriority priority) {
        // Not really supported on Windows Phone
        ::SetThreadPriority(GetCurrentThread(), static_cast<int>(priority) * THREAD_PRIORITY_HIGHEST / static_cast<int>(ThreadPriority::MAXIMUM));
    }

    ThreadUtils::ThreadUtils() {
    }

}
