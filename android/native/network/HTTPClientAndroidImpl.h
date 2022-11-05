/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HTTPCLIENTANDROIDIMPL_H_
#define _CARTO_HTTPCLIENTANDROIDIMPL_H_

#include "network/HTTPClient.h"

#include <atomic>

namespace carto {

    class HTTPClient::AndroidImpl : public HTTPClient::Impl {
    public:
        explicit AndroidImpl(bool log);

        virtual void setTimeout(int milliseconds);
        virtual bool makeRequest(const HTTPClient::Request& request, HeadersFunc headersFn, DataFunc dataFn) const;

    private:
        struct URLClass;
        struct HttpURLConnectionClass;
        struct InputStreamClass;
        struct OutputStreamClass;
        
        static std::unique_ptr<URLClass>& GetURLClass();
        static std::unique_ptr<HttpURLConnectionClass>& GetHttpURLConnectionClass();
        static std::unique_ptr<InputStreamClass>& GetInputStreamClass();
        static std::unique_ptr<OutputStreamClass>& GetOutputStreamClass();

        const bool _log;
        std::atomic<int> _timeout;
    };

}

#endif
