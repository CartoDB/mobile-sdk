/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_HTTPCLIENTPIONIMPL_H_
#define _CARTO_HTTPCLIENTPIONIMPL_H_

#include <asio.hpp>
#include <pion/http/parser.hpp>
#include <pion/http/response.hpp>
#include <pion/tcp/connection.hpp>

#include "network/HTTPClient.h"

namespace carto {

    class HTTPClient::PionImpl : public HTTPClient::Impl {
    public:
        explicit PionImpl(bool log);

        virtual bool makeRequest(const HTTPClient::Request& request, HeaderFn headerFn, DataFn dataFn) const;

    private:
        struct Connection {
            int maxRequests;
            std::chrono::steady_clock::time_point keepAliveTime;
            asio::io_service ioService;
            std::shared_ptr<pion::tcp::connection> connection;

            Connection(const std::string& host, std::uint16_t port);

            bool isValid() const;
        };

        bool makeRequest(Connection& connection, const HTTPClient::Request& request, HeaderFn headerFn, DataFn dataFn) const;

        bool _log;
        mutable std::multimap<std::pair<std::string, int>, std::shared_ptr<Connection> > _connectionMap;
        mutable std::mutex _mutex;
    };

}

#endif
