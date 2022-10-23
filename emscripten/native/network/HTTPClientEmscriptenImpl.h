#ifndef _CARTO_HTTPCLIENTEMSCRIPTENImpl_H_
#define _CARTO_HTTPCLIENTEMSCRIPTENImpl_H_

#include "network/HTTPClient.h"

namespace carto {
    class HTTPClient::EmscriptenImpl : public HTTPClient::Impl {
    public:
        explicit EmscriptenImpl(bool log);

        virtual void setTimeout(int milliseconds);
        virtual bool makeRequest(const HTTPClient::Request& request, HeadersFunc headersFn, DataFunc dataFn) const;

    private:
        bool _log;
        int _timeout;
    };

}

#endif
