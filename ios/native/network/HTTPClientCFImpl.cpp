#include "HTTPClientCFImpl.h"
#include "components/Exceptions.h"
#include "utils/CFUniquePtr.h"
#include "utils/Log.h"

#include <chrono>
#include <limits>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace carto {

    HTTPClient::CFImpl::CFImpl(bool log) :
        _log(log)
    {
    }

    bool HTTPClient::CFImpl::makeRequest(const HTTPClient::Request& request, HeadersFn headersFn, DataFn dataFn) const {
        // Create request
        CFUniquePtr<CFURLRef> cfUrl(CFURLCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(request.url.c_str()), request.url.size(), kCFStringEncodingUTF8, NULL));
        CFUniquePtr<CFStringRef> hostName(CFURLCopyHostName(cfUrl));
        CFUniquePtr<CFStringRef> method(CFStringCreateWithCString(kCFAllocatorDefault, request.method.c_str(), kCFStringEncodingUTF8));
        CFUniquePtr<CFHTTPMessageRef> cfRequest(CFHTTPMessageCreateRequest(NULL, method, cfUrl, kCFHTTPVersion1_1));
        CFHTTPMessageSetHeaderFieldValue(cfRequest, CFSTR("Host"), hostName);
        for (auto it = request.headers.begin(); it != request.headers.end(); it++) {
            CFUniquePtr<CFStringRef> key(CFStringCreateWithCString(kCFAllocatorDefault, it->first.c_str(), kCFStringEncodingUTF8));
            CFUniquePtr<CFStringRef> value(CFStringCreateWithCString(kCFAllocatorDefault, it->second.c_str(), kCFStringEncodingUTF8));
            CFHTTPMessageSetHeaderFieldValue(cfRequest, key, value);
        }

        // Set request body, if contentType defined
        if (!request.contentType.empty()) {
            CFUniquePtr<CFDataRef> data(CFDataCreateWithBytesNoCopy(NULL, request.body.data(), request.body.size(), kCFAllocatorNull));
            CFHTTPMessageSetBody(cfRequest, data);
        }

        CFUniquePtr<CFReadStreamRef> requestStream(CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, cfRequest));
        CFReadStreamSetProperty(requestStream, kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue);
        CFReadStreamSetProperty(requestStream, kCFStreamPropertyHTTPAttemptPersistentConnection, kCFBooleanTrue);
        if (!CFReadStreamOpen(requestStream)) {
            throw NetworkException("Failed to open HTTP stream", request.url);
        }

        // Read initial block of the message. This is needed to parse the headers
        UInt8 buf[4096];
        CFIndex numBytesRead = CFReadStreamRead(requestStream, buf, sizeof(buf));
        if (numBytesRead < 0) {
            throw NetworkException("Failed to read response", request.url);
        }

        // Get response
        CFUniquePtr<CFHTTPMessageRef> cfResponse((CFHTTPMessageRef)CFReadStreamCopyProperty(requestStream, kCFStreamPropertyHTTPResponseHeader));
        if (!cfResponse) {
            throw NetworkException("Failed to read HTTP headers", request.url);
        }

        int statusCode = static_cast<int>(CFHTTPMessageGetResponseStatusCode(cfResponse));

        std::map<std::string, std::string> headers;
        CFUniquePtr<CFDictionaryRef> headersDict(CFHTTPMessageCopyAllHeaderFields(cfResponse));
        std::size_t headersCount = CFDictionaryGetCount(headersDict);
        std::vector<CFTypeRef> headerKeys(headersCount);
        std::vector<CFTypeRef> headerValues(headersCount);
        CFDictionaryGetKeysAndValues(headersDict, headerKeys.data(), headerValues.data());
        for (std::size_t i = 0; i < headersCount; i++) {
            CFStringRef headerKey = (CFStringRef)headerKeys[i];
            std::vector<char> key(CFStringGetLength(headerKey) * 6 + 1);
            CFStringGetCString(headerKey, key.data(), key.size(), kCFStringEncodingUTF8);

            CFStringRef headerValue = (CFStringRef)headerValues[i];
            std::vector<char> value(CFStringGetLength(headerValue) * 6 + 1);
            CFStringGetCString(headerValue, value.data(), value.size(), kCFStringEncodingUTF8);

            headers[key.data()] = value.data();
        }

        bool cancel = false;
        if (!headersFn(statusCode, headers)) {
            cancel = true;
        }
        else {
            if (numBytesRead > 0) {
                if (!dataFn(static_cast<const unsigned char*>(&buf[0]), numBytesRead)) {
                    cancel = true;
                }
            }
        }

        // Read Content-Length
        std::uint64_t contentLength = std::numeric_limits<std::uint64_t>::max();
        auto it = headers.find("Content-Length");
        if (it != headers.end()) {
            contentLength = boost::lexical_cast<std::uint64_t>(it->second);
        }

        // Read message body
        for (std::uint64_t offset = numBytesRead; offset < contentLength && !cancel; ) {
            numBytesRead = CFReadStreamRead(requestStream, buf, sizeof(buf));
            if (numBytesRead < 0) {
                throw NetworkException("Failed to read data", request.url);
            }
            else if (numBytesRead == 0) {
                if (contentLength == std::numeric_limits<std::uint64_t>::max()) {
                    break;
                }
                throw NetworkException("Failed to read full data", request.url);
            }

            if (!dataFn(static_cast<const unsigned char*>(&buf[0]), numBytesRead)) {
                cancel = true;
            }

            offset += numBytesRead;
        }

        // Done
        CFReadStreamClose(requestStream);

        return !cancel;
    }

}
