/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CANCELABLETHREADPOOL_H_
#define _CARTO_CANCELABLETHREADPOOL_H_

#include "components/CancelableTask.h"
#include "components/ThreadWorker.h"

#include <condition_variable>
#include <memory>
#include <queue>
#include <thread>

namespace carto {

    class CancelableThreadPool : public std::enable_shared_from_this<CancelableThreadPool> {
    public:
        CancelableThreadPool();
        virtual ~CancelableThreadPool();
        void deinit();
    
        int getPoolSize() const;
        void setPoolSize(int threadCount);
    
        void execute(std::shared_ptr<CancelableTask>);
        void execute(std::shared_ptr<CancelableTask>, int priority);
    
        void cancelAll();
        
    private:
        struct TaskRecord {
            TaskRecord(std::shared_ptr<CancelableTask> task, int priority, long long sequence);
    
            bool operator <(const TaskRecord& taskRecord) const;
    
            std::shared_ptr<CancelableTask> _task;
            int _priority;
            long long _sequence;
        };
    
        struct TaskWorker : public ThreadWorker {
            TaskWorker(const std::shared_ptr<CancelableThreadPool>& threadPool, int priority);
    
            void operator()();
    
            std::weak_ptr<CancelableThreadPool> _threadPool;
            int _priority; // mutable, guarded by _threadPool->_mutex
        };
    
        bool getNextTask(std::shared_ptr<CancelableTask>& task, int priority);
    
        bool shouldTerminateWorker(TaskWorker& worker);
    
        static const int DEFAULT_PRIORITY;
    
        int _poolSize;
        long long _taskCount;
        bool _stop;
    
        std::priority_queue<TaskRecord> _taskRecords;
        std::vector<std::shared_ptr<TaskWorker> > _workers;
        std::vector<std::thread> _threads;
    
        std::condition_variable _condition;
        mutable std::mutex _mutex;
    };
    
}

#endif
