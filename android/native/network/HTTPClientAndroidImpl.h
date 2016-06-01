/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HTTPCLIENTANDROIDIMPL_H_
#define _CARTO_HTTPCLIENTANDROIDIMPL_H_

#include "network/HTTPClient.h"

namespace carto {

    class HTTPClient::AndroidImpl : public HTTPClient::Impl {
    public:
        explicit AndroidImpl(bool log);

        virtual bool makeRequest(const HTTPClient::Request& request, HeadersFn headersFn, DataFn dataFn) const;

    private:
        struct URLClass;
        struct HttpURLConnectionClass;
        struct BufferedInputStreamClass;
        struct OutputStreamClass;
        
        static std::unique_ptr<URLClass> _URLClass;
        static std::unique_ptr<HttpURLConnectionClass> _HttpURLConnectionClass;
        static std::unique_ptr<BufferedInputStreamClass> _BufferedInputStreamClass;
        static std::unique_ptr<OutputStreamClass> _OutputStreamClass;
        static std::mutex _Mutex;

        bool _log;
    };

}

#endif
