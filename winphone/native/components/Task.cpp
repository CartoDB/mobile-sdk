#include "components/Task.h"

#include <wrl.h>
#include <wrl/implements.h>
#include <concrt.h>
#include <ppltasks.h>
#include <windows.h>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::System::Threading;

namespace carto {

    void Task::operator()() {
        HANDLE eventHandle = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
        
        auto workItemHandler = ref new WorkItemHandler([this, eventHandle](IAsyncAction^ action) {
            run();
            SetEvent(eventHandle);
        }, CallbackContext::Any);
        ThreadPool::RunAsync(workItemHandler, WorkItemPriority::Low, WorkItemOptions::TimeSliced);
        
        WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
        CloseHandle(eventHandle);
    }
    
}
