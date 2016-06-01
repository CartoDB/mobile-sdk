#include "utils/ThreadUtils.h"
#include "components/ThreadWorker.h"
#include "utils/Log.h"

#include <sys/syscall.h>

#import <Foundation/NSThread.h>

namespace carto {

    void ThreadUtils::SetThreadPriority(ThreadPriority::ThreadPriority priority) {
    	NSThread * nsThread = [NSThread currentThread];
        float nsPriority = 1 - (priority + 20) / 40.0;
        [nsThread setThreadPriority:nsPriority];
    }
    
    ThreadUtils::ThreadUtils() {
    }

}
