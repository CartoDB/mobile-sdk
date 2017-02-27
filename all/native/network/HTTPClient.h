/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HTTPCLIENT_H_
#define _CARTO_HTTPCLIENT_H_

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <cstdint>
#include <functional>

namespace carto {
    class BinaryData;

    class HTTPClient {
    public:
        typedef std::function<bool(std::uint64_t, std::uint64_t, const unsigned char*, std::size_t)> HandlerFn;

        explicit HTTPClient(bool log);

        void setTimeout(int milliseconds);

        int get(const std::string& url, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, std::shared_ptr<BinaryData>& responseData, int* statusCode = 0) const;
        int get(const std::string& url, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, HandlerFn handlerFn, std::uint64_t offset) const;
        int post(const std::string& url, const std::string& contentType, const std::shared_ptr<BinaryData>& requestData, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, std::shared_ptr<BinaryData>& responseData);

    private:
        struct HeaderLess {
            bool operator() (const std::string& header1, const std::string& header2) const {
                return std::lexicographical_compare(header1.begin(), header1.end(), header2.begin(), header2.end(), [](char c1, char c2) {
                    return uncase(c1) < uncase(c2);
                });
            }

        private:
            static char uncase(char c) {
                return c >= 'A' && c <= 'Z' ? (c - 'A' + 'a') : c;
            }
        };

        struct Request {
            std::string url;
            std::string method;
            std::map<std::string, std::string, HeaderLess> headers;
            std::string contentType;
            std::vector<unsigned char> body;

            explicit Request(const std::string& method, const std::string& url) : url(url), method(method) { }
        };

        struct Response {
            int statusCode = -1;
            std::map<std::string, std::string, HeaderLess> headers;
            std::vector<unsigned char> body;

            Response() { }
            explicit Response(int statusCode) : statusCode(statusCode) { }
        };

        class Impl {
        public:
            typedef std::function<bool(int, const std::map<std::string, std::string>&)> HeadersFn;
            typedef std::function<bool(const unsigned char*, std::size_t)> DataFn;

            virtual ~Impl();

            virtual void setTimeout(int milliseconds) = 0;
            virtual bool makeRequest(const HTTPClient::Request& request, HeadersFn headersFn, DataFn dataFn) const = 0;
        };

        class PionImpl;
        class AndroidImpl;
        class CFImpl;
        class WinSockImpl;

        int makeRequest(Request request, Response& response, HandlerFn handlerFn, std::uint64_t offset) const;

        bool _log;
        std::unique_ptr<Impl> _impl;
    };

}

#endif
