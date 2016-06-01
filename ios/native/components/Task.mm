#include "components/Task.h"

namespace carto {

    // Autorelease pool doesn't compile without an .mm file
    void Task::operator()() {
        @autoreleasepool {
            run();
        }
    }
    
}
