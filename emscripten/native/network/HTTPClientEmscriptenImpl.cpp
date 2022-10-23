#include "HTTPClientEmscriptenImpl.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <emscripten/fetch.h>

namespace carto {
    HTTPClient::EmscriptenImpl::EmscriptenImpl(bool log) :
        _log(log),
        _timeout(-1)
    {
    }

    void HTTPClient::EmscriptenImpl::setTimeout(int milliseconds) {
        _timeout = milliseconds;
    }

    bool HTTPClient::EmscriptenImpl::makeRequest(const HTTPClient::Request& request, HeadersFunc headersFn, DataFunc dataFn) const {
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, request.method.c_str());
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
        attr.timeoutMSecs = _timeout;

        const char* headersArray[request.headers.size() * 2 + 1];
        int i = 0;
        for (const auto& [key, value] : request.headers) {
            headersArray[i++] = key.c_str();
            headersArray[i++] = value.c_str();
        }
        headersArray[i] = NULL;
        attr.requestHeaders = headersArray;

        emscripten_fetch_t *fetch = emscripten_fetch(&attr, request.url.c_str());

        bool cancel = false;
        std::map<std::string, std::string> headers;

        if (!headersFn(fetch->status, headers)) {
            cancel = true;
        }

        if (!cancel) {
            if(!dataFn((const unsigned char *)fetch->data, fetch->numBytes)) {
                cancel = true;
            }
        }

        emscripten_fetch_close(fetch);

        return !cancel;
    }
}
