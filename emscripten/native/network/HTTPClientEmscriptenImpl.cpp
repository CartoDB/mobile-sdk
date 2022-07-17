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
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
        attr.timeoutMSecs = _timeout;
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
