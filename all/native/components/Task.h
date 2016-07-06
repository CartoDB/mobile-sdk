/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TASK_H_
#define _CARTO_TASK_H_

#include <memory>

namespace carto {

    class Task : public std::enable_shared_from_this<Task> {
    public:
        virtual ~Task() { }

        void operator()();

    protected:
        Task() { }

        virtual void run() = 0;
    };

}

#endif
