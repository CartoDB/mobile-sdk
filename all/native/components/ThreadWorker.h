/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_THREADWORKER_H_
#define _CARTO_THREADWORKER_H_

namespace carto {

    class ThreadWorker {
    public:
        virtual ~ThreadWorker() { }
    
        virtual void operator()() = 0;
    
    protected:
        ThreadWorker() { }
    };
    
}

#endif
