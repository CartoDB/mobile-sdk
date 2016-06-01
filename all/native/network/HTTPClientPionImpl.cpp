#include "HTTPClientPionImpl.h"
#include "utils/Log.h"

#include <chrono>
#include <limits>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace carto {

    HTTPClient::PionImpl::PionImpl(bool log) :
        _log(log), _connectionMap(), _mutex()
    {
    }

    bool HTTPClient::PionImpl::makeRequest(const HTTPClient::Request& request, HeaderFn headerFn, DataFn dataFn) const {
        // Parse request URL
        std::string proto, host, path, query;
        std::uint16_t port;
        if (!pion::http::parser::parse_uri(request.url, proto, host, port, path, query)) {
            if (_log) {
                Log::Errorf("HTTPClient::PionImpl::makeRequest: Failed to parse URL: %s", request.url.c_str());
            }
            return false;
        }
        if (proto == "https") {
            if (_log) {
                Log::Error("HTTPClient::PionImpl::makeRequest: Protocol not supported: https");
            }
            return false;
        }
        auto connectionKey = std::make_pair(host, port);

        // Try to reuse existing connection from the pool for GET methods
        bool result = false;
        while (true) {
            std::shared_ptr<Connection> connection;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                auto it = _connectionMap.find(connectionKey);
                if (it != _connectionMap.end()) {
                    connection = it->second;
                    _connectionMap.erase(it);
                }
            }
            if (!connection) {
                break;
            }
            if (!connection->isValid()) {
                continue;
            }

            try {
                result = makeRequest(*connection, request, headerFn, dataFn);
            }
            catch (const std::exception& ex) {
                if (_log) {
                    Log::Errorf("HTTPClient::PionImpl::makeRequest: Exception: %s", ex.what());
                }
            }
            if (result) {
                std::lock_guard<std::mutex> lock(_mutex);
                if (request.method == "GET") {
                    _connectionMap.insert(std::make_pair(connectionKey, connection));
                }
                return result;
            }
        }

        // Create new connection
        auto connection = std::make_shared<Connection>(host, port);
        if (!connection->isValid()) {
            return false;
        }

        try {
            result = makeRequest(*connection, request, headerFn, dataFn);
        }
        catch (const std::exception& ex) {
            if (_log) {
                Log::Errorf("HTTPClient::PionImpl::makeRequest: Exception: %s", ex.what());
            }
        }
        if (result) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (request.method == "GET") {
                _connectionMap.insert(std::make_pair(connectionKey, connection));
            }
        }
        return result;
    }

    bool HTTPClient::PionImpl::makeRequest(Connection& connection, const HTTPClient::Request& request, HeaderFn headerFn, DataFn dataFn) const {
        std::string url = request.url;
        std::string proto, host, path, query;
        std::uint16_t port;
        if (!pion::http::parser::parse_uri(request.url, proto, host, port, path, query)) {
            if (_log) {
                Log::Errorf("HTTPClient::PionImpl::makeRequest: Failed to parse URL: %s", url.c_str());
                return false;
            }
        }

        // Form and send request
        asio::error_code socketError;
        std::chrono::steady_clock::time_point requestTime = std::chrono::steady_clock::now();
        pion::http::request pionRequest(path);
        pionRequest.set_method(request.method);
        pionRequest.set_query_string(query);
        if (!request.contentType.empty()) {
            pionRequest.set_content(reinterpret_cast<const char*>(request.body.data()), request.body.size());
        }
        else {
            pionRequest.set_do_not_send_content_length();
        }
        pionRequest.add_header("Host", host);
        for (auto it = request.headers.begin(); it != request.headers.end(); it++) {
            pionRequest.add_header(it->first, it->second);
        }
        pionRequest.send(*connection.connection, socketError);
        if (socketError) {
            return false;
        }

        // Send the request
        bool cancel = false;
        asio::error_code parserError;
        pion::http::parser parser(false, 0);
        parser.set_payload_handler([&dataFn, &cancel](const char* buf, std::size_t size) {
            if (!dataFn(reinterpret_cast<const unsigned char*>(buf), size)) {
                cancel = true;
            }
        });

        // Read headers
        asio::streambuf buffer;
        std::size_t bytesRead = asio::read_until(*connection.connection, buffer, "\r\n\r\n", socketError);
        if (socketError) {
            if (_log) {
                std::string message = socketError.message();
                Log::Errorf("HTTPClient::PionImpl::makeRequest: Socket error: %s, URL: %s", message.c_str(), url.c_str());
            }
            return false;
        }

        // Feed read data to HTTP parser
        pion::http::response pionResponse;
        std::vector<char> bufferData(asio::buffers_begin(buffer.data()), asio::buffers_end(buffer.data()));
        parser.set_read_buffer(bufferData.data(), bufferData.size());
        parser.parse(pionResponse, parserError);
        buffer.consume(bufferData.size());
        if (parserError) {
            if (_log) {
                Log::Errorf("HTTPClient::PionImpl::makeRequest: HTTP parsing error, URL: %s", url.c_str());
            }
            return false;
        }

        // Call headers callback
        std::map<std::string, std::string> headers;
        headers.insert(pionResponse.get_headers().begin(), pionResponse.get_headers().end());
        if (!headersFn(pionResponse.get_status_code(), headers)) {
            cancel = true;
        }

        // Check Keep-Alive directive
        connection.maxRequests--;
        it = pionResponse.get_headers().find("Keep-Alive");
        if (it != pionResponse.get_headers().end()) {
            std::cmatch what;
            if (std::regex_match(it->second.c_str(), what, std::regex(".*[^a-zA-Z0-9]timeout=([0-9]*).*"))) {
                long long timeout = boost::lexical_cast<long long>(what[1]);
                connection.keepAliveTime = requestTime + std::chrono::seconds(timeout);
            }
            if (std::regex_match(it->second.c_str(), what, std::regex(".*[^a-zA-Z0-9]max=([0-9]*).*"))) {
                int maxRequests = boost::lexical_cast<int>(what[1]);
                connection.maxRequests = std::min(connection.maxRequests, maxRequests);
            }
        }
        else {
            connection.keepAliveTime = requestTime + std::chrono::seconds(5); // Apache servers have this limitation typically
        }

        // Read Content-Length
        std::uint64_t contentLength = std::numeric_limits<std::uint64_t>::max();
        auto it = pionResponse.get_headers().find("Content-Length");
        if (it != pionResponse.get_headers().end()) {
            contentLength = boost::lexical_cast<std::uint64_t>(it->second);
        }
        else {
            connection.maxRequests = 0; // force new connection next time
        }

        // Read response
        for (std::uint64_t offset = 0; offset < contentLength && !cancel; ) {
            bytesRead = asio::read(*connection.connection, buffer, asio::transfer_at_least(1), socketError);
            if (socketError) {
                // If Content-Length was not explicitly defined, break at EOF
                if (socketError == asio::error::eof && contentLength == std::numeric_limits<std::uint64_t>::max()) {
                    break;
                }
                if (_log) {
                    std::string message = socketError.message();
                    Log::Errorf("HTTPClient::PionImpl::makeRequest: Socket error: %s, URL: %s", message.c_str(), url.c_str());
                }
                return false;
            }

            // Parse read data
            bufferData.assign(asio::buffers_begin(buffer.data()), asio::buffers_end(buffer.data()));
            parser.set_read_buffer(bufferData.data(), bufferData.size());
            parser.parse(pionResponse, parserError);
            buffer.consume(bufferData.size());
            if (parserError) {
                if (_log) {
                    Log::Errorf("HTTPClient::PionImpl::makeRequest: HTTP parsing error, URL: %s", url.c_str());
                }
                return false;
            }

            offset += bytesRead;
        }

        return !cancel;
    }

    HTTPClient::PionImpl::Connection::Connection(const std::string& host, std::uint16_t port) :
        maxRequests(std::numeric_limits<int>::max()), keepAliveTime(), ioService(), connection()
    {
        // Connect to server
        connection = std::make_shared<pion::tcp::connection>(ioService);
        connection->set_lifecycle(pion::tcp::connection::LIFECYCLE_KEEPALIVE);
        asio::error_code socketError = connection->connect(host, port);
        if (socketError) {
            connection.reset();
        }
    }

    bool HTTPClient::PionImpl::Connection::isValid() const {
        if (!connection) {
            return false;
        }
        if (connection->get_lifecycle() == pion::tcp::connection::LIFECYCLE_CLOSE) {
            return false;
        }
        std::chrono::steady_clock::time_point nullTime;
        return maxRequests > 0 && (keepAliveTime == nullTime || keepAliveTime > std::chrono::steady_clock::now());
    }

}
