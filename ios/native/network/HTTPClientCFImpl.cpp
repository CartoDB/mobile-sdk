#include "HTTPClientCFImpl.h"
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
        if (!CFReadStreamOpen(requestStream)) {
            if (_log) {
                Log::Errorf("HTTPClient::CFImpl::makeRequest: Failed to open HTTP stream: %s", request.url.c_str());
            }
            return false;
        }

        // Read initial block of the message. This is needed to parse the headers
        UInt8 buf[4096];
        CFIndex numBytesRead = CFReadStreamRead(requestStream, buf, sizeof(buf));
        if (numBytesRead < 0) {
            if (_log) {
                Log::Errorf("HTTPClient::CFImpl::makeRequest: Failed to read response: URL: %s", request.url.c_str());
            }
            return false;
        }

        // Get response
        CFUniquePtr<CFHTTPMessageRef> cfResponse((CFHTTPMessageRef)CFReadStreamCopyProperty(requestStream, kCFStreamPropertyHTTPResponseHeader));
        if (!cfResponse) {
            if (_log) {
                Log::Errorf("HTTPClient::CFImpl::makeRequest: Failed to read HTTP stream headers: %s", request.url.c_str());
            }
            return false;
        }


        int statusCode = CFHTTPMessageGetResponseStatusCode(cfResponse);

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
                if (_log) {
                    Log::Errorf("HTTPClient::CFImpl::makeRequest: Failed to read response: URL: %s", request.url.c_str());
                }
                return false;
            }
            else if (numBytesRead == 0) {
                if (contentLength != std::numeric_limits<std::uint64_t>::max()) {
                    if (_log) {
                        Log::Errorf("HTTPClient::CFImpl::makeRequest: Failed to read full response: URL: %s", request.url.c_str());
                    }
                    return false;
                }
                break;
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
