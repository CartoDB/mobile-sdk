#include "HTTPClient.h"
#include "core/BinaryData.h"
#include "utils/Log.h"

#include <chrono>
#include <limits>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#if defined(_WIN32)
#define CARTO_HTTP_SOCKET_IMPL WinSockImpl
#include "network/HTTPClientWinSockImpl.h"
#elif defined(__APPLE__)
#define CARTO_HTTP_SOCKET_IMPL CFImpl
#include "network/HTTPClientCFImpl.h"
#elif defined(__ANDROID__)
#define CARTO_HTTP_SOCKET_IMPL AndroidImpl
#include "network/HTTPClientAndroidImpl.h"
#else
#define CARTO_HTTP_SOCKET_IMPL PionImpl
#include "HTTPClientPionImpl.h"
#endif

namespace carto {

    HTTPClient::HTTPClient(bool log) :
        _log(log), _impl(new CARTO_HTTP_SOCKET_IMPL(log))
    {
    }

    int HTTPClient::get(const std::string& url, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, std::shared_ptr<BinaryData>& responseData) const {
        Request request("GET", url);
        request.headers.insert(requestHeaders.begin(), requestHeaders.end());
        if (request.headers.count("Accept") == 0) {
            request.headers["Accept"] = "*/*";
        }

        std::vector<unsigned char> content;
        content.reserve(65536);
        auto handlerFn = [&content](std::uint64_t offset, std::uint64_t length, const unsigned char* buf, std::size_t size) -> bool {
            if (content.size() != offset) {
                content.resize(static_cast<size_t>(offset));
            }
            content.insert(content.end(), buf, buf + size);
            return true;
        };

        Response response;
        int code = makeRequest(request, response, handlerFn, 0);
        responseHeaders.insert(response.headers.begin(), response.headers.end());
        responseData = std::make_shared<BinaryData>(std::move(content));
        return code;
    }

    int HTTPClient::get(const std::string& url, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, HandlerFn handlerFn, std::uint64_t offset) const {
        Request request("GET", url);
        request.headers.insert(requestHeaders.begin(), requestHeaders.end());
        if (request.headers.count("Accept") == 0) {
            request.headers["Accept"] = "*/*";
        }
        if (offset > 0) {
            request.headers["Range"] = "bytes=" + boost::lexical_cast<std::string>(offset) + "-";
        }

        Response response;
        int code = makeRequest(request, response, handlerFn, offset);
        responseHeaders.insert(response.headers.begin(), response.headers.end());
        return code;
    }

    int HTTPClient::post(const std::string& url, const std::string& contentType, const std::shared_ptr<BinaryData>& requestData, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, std::shared_ptr<BinaryData>& responseData) {
        Request request("POST", url);
        request.contentType = contentType;
        request.headers["Content-Type"] = contentType;
        request.headers.insert(requestHeaders.begin(), requestHeaders.end());
        if (request.headers.count("Accept") == 0) {
            request.headers["Accept"] = "*/*";
        }
        request.body = *requestData->getDataPtr();
        
        std::vector<unsigned char> content;
        content.reserve(65536);
        auto handlerFn = [&content](std::uint64_t offset, std::uint64_t length, const unsigned char* buf, std::size_t size) -> bool {
            if (content.size() != offset) {
                content.resize(static_cast<size_t>(offset));
            }
            content.insert(content.end(), buf, buf + size);
            return true;
        };

        Response response;
        int code = makeRequest(request, response, handlerFn, 0);
        responseHeaders.insert(response.headers.begin(), response.headers.end());
        responseData = std::make_shared<BinaryData>(std::move(content));
        return code;
    }

    int HTTPClient::makeRequest(Request request, Response& response, HandlerFn handlerFn, std::uint64_t offset) const {
        std::uint64_t contentOffset = 0;
        std::uint64_t contentLength = std::numeric_limits<std::uint64_t>::max();

        auto headersFn = [&](int statusCode, const std::map<std::string, std::string>& headers) {
            response.statusCode = statusCode;
            response.headers.insert(headers.begin(), headers.end());

            // Read Content-Range
            if (statusCode == 206) {
                auto it = response.headers.find("Content-Range");
                if (it != response.headers.end()) {
                    std::cmatch what;
                    if (std::regex_match(it->second.c_str(), what, std::regex("bytes ([0-9]+)-.*"))) {
                        contentOffset = boost::lexical_cast<std::uint64_t>(what[1]);
                    }
                }
                if (contentOffset != offset) {
                    if (_log) {
                        Log::Errorf("HTTPClient::makeRequest: Content range mismatch: URL: %s", request.url.c_str());
                    }
                    response = Response();
                    return false;
                }
            }

            // Read Content-Length
            auto it = response.headers.find("Content-Length");
            if (it != response.headers.end()) {
                contentLength = boost::lexical_cast<std::uint64_t>(it->second);
            }

            return true;
        };

        auto dataFn = [&](const unsigned char* data, std::size_t size) {
            bool result = handlerFn(offset, contentOffset + contentLength, data, size);
            offset += size;
            return result;
        };

        if (!_impl->makeRequest(request, headersFn, dataFn)) {
            return -1;
        }

        if (response.statusCode >= 300 && response.statusCode < 400) {
            auto it = response.headers.find("Location");
            if (it != response.headers.end()) {
                std::string location = it->second;
                if (_log) {
                    Log::Infof("HTTPClient::makeRequest: Redirection from URL: %s to URL: %s", request.url.c_str(), location.c_str());
                }
                Request redirectedRequest(request);
                redirectedRequest.url = location;
                response = Response();
                return makeRequest(redirectedRequest, response, handlerFn, offset);
            }
        }

        if (response.statusCode < 200 || response.statusCode >= 300) {
            if (_log) {
                Log::Errorf("HTTPClient::makeRequest: Bad status code: %d, URL: %s", response.statusCode, request.url.c_str());
            }
            return response.statusCode;
        }

        return 0;
    }

    HTTPClient::Impl::~Impl() {
    }

}
