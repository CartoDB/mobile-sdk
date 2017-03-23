/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HTTPCLIENTCFIMPL_H_
#define _CARTO_HTTPCLIENTCFIMPL_H_

#include <CFNetwork/CFNetwork.h>

#include "network/HTTPClient.h"

namespace carto {

    class HTTPClient::CFImpl : public HTTPClient::Impl {
    public:
        explicit CFImpl(bool log);

        virtual void setTimeout(int milliseconds);
        virtual bool makeRequest(const HTTPClient::Request& request, HeadersFn headersFn, DataFn dataFn) const;

    private:
        bool _log;
        int _timeout;
    };

}

#endif
