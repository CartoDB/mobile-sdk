#include "utils/ThreadUtils.h"
#include "components/ThreadWorker.h"
#include "utils/Log.h"

#include <sys/syscall.h>

#import <Foundation/NSThread.h>

namespace carto {

    void ThreadUtils::SetThreadPriority(ThreadPriority::ThreadPriority priority) {
        NSThread* nsThread = [NSThread currentThread];
        double nsPriority = 1 - static_cast<double>(static_cast<int>(priority) - static_cast<int>(ThreadPriority::MAXIMUM)) / (static_cast<int>(ThreadPriority::MINIMUM) - static_cast<int>(ThreadPriority::MAXIMUM));
        [nsThread setThreadPriority:nsPriority];
    }
    
    ThreadUtils::ThreadUtils() {
    }

}
