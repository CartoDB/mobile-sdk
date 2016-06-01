#include "CancelableThreadPool.h"
#include "utils/Log.h"
#include "utils/ThreadUtils.h"

#include "core/MapPos.h"

namespace carto {

    CancelableThreadPool::CancelableThreadPool() :
        _poolSize(0),
        _taskCount(0),
        _stop(false),
        _taskRecords(),
        _workers(),
        _threads(),
        _mutex()
    {
    }
    
    CancelableThreadPool::~CancelableThreadPool() {
    }
        
    void CancelableThreadPool::deinit() {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _stop = true;
        }
        
        cancelAll();
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _condition.notify_all();
        }
        
        for (const std::shared_ptr<std::thread>& thread : _threads) {
            thread->detach();
        }
        
        _workers.clear();
        _threads.clear();
    }
    
    int CancelableThreadPool::getPoolSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _poolSize;
    }
    
    void CancelableThreadPool::setPoolSize(int poolSize) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (_stop) {
            return;
        }
    
        // Add threads
        for (int i = _poolSize; i < poolSize; i++) {
            _workers.push_back(std::make_shared<TaskWorker>(shared_from_this()));
            _threads.push_back(std::make_shared<std::thread>(&TaskWorker::operator(), _workers.back()));
        }
    
        _poolSize = poolSize;
    }
    
    void CancelableThreadPool::execute(std::shared_ptr<CancelableTask> task) {
        execute(task, DEFAULT_PRIORITY);
    }
    
    void CancelableThreadPool::execute(std::shared_ptr<CancelableTask> task, int priority) {
        if (!task->isCanceled()) {
            std::lock_guard<std::mutex> lock(_mutex);
            
            if (_stop) {
                return;
            }
    
            // Push task to queue, increase global task count
            _taskRecords.push(TaskRecord(task, priority, _taskCount));
            _taskCount++;
    
            // If there are any waiting threads, notify one of them
            _condition.notify_one();
        }
    }
    
    void CancelableThreadPool::cancelAll() {
        std::lock_guard<std::mutex> lock(_mutex);
        
        size_t taskRecordsSize = _taskRecords.size();
        for (size_t i = 0; i < taskRecordsSize; i++) {
            const std::shared_ptr<CancelableTask>& task = _taskRecords.top()._task;
            task->cancel();
            _taskRecords.pop();
        }
    }
    
    CancelableThreadPool::TaskRecord::TaskRecord(std::shared_ptr<CancelableTask> task, int priority, long long sequence) :
            _task(task),
            _priority(priority),
            _sequence(sequence) {
    }
    
    bool CancelableThreadPool::TaskRecord::operator <(const TaskRecord& taskRecord) const {
        // Tasks are sorted according to their priority and then their sequence
        if (_priority != taskRecord._priority) {
            return _priority < taskRecord._priority;
        }
        return _sequence > taskRecord._sequence;
    }
    
    CancelableThreadPool::TaskWorker::TaskWorker(const std::shared_ptr<CancelableThreadPool>& threadPool) :
        _threadPool(threadPool) {
    }
        
    void CancelableThreadPool::TaskWorker::operator ()() {
        ThreadUtils::SetThreadPriority(ThreadPriority::MINIMUM);
        while (true) {
            auto threadPool = _threadPool.lock();
            if (!threadPool) {
                return;
            }

            // If there are no tasks, wait until notified or exit thread if interrupted
            {
                std::unique_lock<std::mutex> lock(threadPool->_mutex);
                if (threadPool->_stop) {
                    return;
                }
                
                if (threadPool->_taskRecords.size() == 0) {
                    threadPool->_condition.wait(lock);
                }
            }
    
            // Request another task, execute it if it's not null
            while (true) {
                {
                    std::lock_guard<std::mutex> lock(threadPool->_mutex);
                    if (threadPool->_stop) {
                        return;
                    }
                }
                
                std::shared_ptr<CancelableTask> task = threadPool->getNextTask();
                if (task) {
                    task->operator ()();
                } else {
                    break;
                }
    
                if (threadPool->shouldTerminateWorker(*this)) {
                    return;
                }
    
                // Check for interruption
                std::this_thread::yield();
            }
        }
    }
    
    std::shared_ptr<CancelableTask> CancelableThreadPool::getNextTask() {
        std::lock_guard<std::mutex> lock(_mutex);
    
        // Return the next highest priority task from the task queue
        std::shared_ptr<CancelableTask> task;
        if (_taskRecords.size() > 0) {
            task = _taskRecords.top()._task;
            _taskRecords.pop();
        }
        return task;
    }
    
    bool CancelableThreadPool::shouldTerminateWorker(TaskWorker& worker) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (_stop) {
            return true;
        }
    
        // If there are too many threads, remove this worker and it's thread
        if ((int) _threads.size() >_poolSize) {
    
            // Find the index of the finished worker, it's thread will have the same index in _threads vector
            int index = 0;
            WorkerList::iterator it;
            for (it = _workers.begin(); it != _workers.end(); ++it) {
                const std::shared_ptr<TaskWorker>& listWorker = *it;
                if (listWorker.get() == &worker) {
                    // Remove thread and worker
                    _workers.erase(it);
                    _threads.erase(_threads.begin() + index);
                    break;
                }
                index++;
            }
    
            return true;
        }
    
        return false;
    }
    
}
