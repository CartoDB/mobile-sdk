/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HTTPCLIENTWINSOCKIMPL_H_
#define _CARTO_HTTPCLIENTWINSOCKIMPL_H_

#include "network/HTTPClient.h"

#include <atomic>

namespace carto {

    class HTTPClient::WinSockImpl : public HTTPClient::Impl {
    public:
        explicit WinSockImpl(bool log);

        virtual void setTimeout(int milliseconds);
        virtual bool makeRequest(const HTTPClient::Request& request, HeadersFunc headersFn, DataFunc dataFn) const;

    private:
        const bool _log;
        std::atomic<int> _timeout;
    };

}

#endif
