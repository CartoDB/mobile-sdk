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
        _log(log),
        _timeout(-1)
    {
    }

    void HTTPClient::CFImpl::setTimeout(int milliseconds) {
        _timeout = milliseconds;
    }

    bool HTTPClient::CFImpl::makeRequest(const HTTPClient::Request& request, HeadersFunc headersFn, DataFunc dataFn) const {
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

        // Configure connection parameters
        CFUniquePtr<CFReadStreamRef> requestStream(CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, cfRequest));
        CFReadStreamSetProperty(requestStream, kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue);
        CFReadStreamSetProperty(requestStream, kCFStreamPropertyHTTPAttemptPersistentConnection, kCFBooleanTrue);

#ifdef _CARTO_IGNORE_SSL_CERTS
        CFTypeRef sslKeys[1] = { kCFStreamSSLValidatesCertificateChain };
        CFTypeRef sslValues[1] = { kCFBooleanFalse };
        CFUniquePtr<CFDictionaryRef> sslDict(CFDictionaryCreate(NULL, (const void**)sslKeys, (const void**)sslValues, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
        CFReadStreamSetProperty(requestStream, kCFStreamPropertySSLSettings, sslDict);
#endif

        struct StreamState {
            bool headersReceived;
            bool cancel;
            std::string error;
            HeadersFunc headersFn;
            DataFunc dataFn;
        } streamState { false, false, std::string(), headersFn, dataFn };

        CFOptionFlags registeredEvents = kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered;
        CFStreamClientContext streamContext = { 0, &streamState, NULL, NULL, NULL };

        if (!CFReadStreamSetClient(requestStream, registeredEvents, [](CFReadStreamRef requestStream, CFStreamEventType event, void* ptr) {
            StreamState& streamState = *static_cast<StreamState*>(ptr);
            UInt8 buf[4096];
            CFIndex numBytesRead = 0;

            switch (event) {
            case kCFStreamEventHasBytesAvailable:
                // It is safe to call CFReadStreamRead; it won't block because bytes are available.
                numBytesRead = CFReadStreamRead(requestStream, buf, sizeof(buf));

                // Headers callback
                if (!streamState.headersReceived) {
                    streamState.headersReceived = true;

                    // Get response
                    CFUniquePtr<CFHTTPMessageRef> cfResponse((CFHTTPMessageRef)CFReadStreamCopyProperty(requestStream, kCFStreamPropertyHTTPResponseHeader));
                    if (!cfResponse) {
                        streamState.error = "Failed to read HTTP headers";
                        CFRunLoopStop(CFRunLoopGetCurrent());
                        break;
                    }

                    int statusCode = static_cast<int>(CFHTTPMessageGetResponseStatusCode(cfResponse));

                    CFUniquePtr<CFDictionaryRef> headersDict(CFHTTPMessageCopyAllHeaderFields(cfResponse));
                    std::size_t headersCount = CFDictionaryGetCount(headersDict);
                    std::vector<CFTypeRef> headerKeys(headersCount);
                    std::vector<CFTypeRef> headerValues(headersCount);
                    CFDictionaryGetKeysAndValues(headersDict, headerKeys.data(), headerValues.data());
                    std::map<std::string, std::string> headers;
                    for (std::size_t i = 0; i < headersCount; i++) {
                        CFStringRef headerKey = (CFStringRef)headerKeys[i];
                        std::vector<char> key(CFStringGetLength(headerKey) * 6 + 1);
                        CFStringGetCString(headerKey, key.data(), key.size(), kCFStringEncodingUTF8);

                        CFStringRef headerValue = (CFStringRef)headerValues[i];
                        std::vector<char> value(CFStringGetLength(headerValue) * 6 + 1);
                        CFStringGetCString(headerValue, value.data(), value.size(), kCFStringEncodingUTF8);

                        headers[key.data()] = value.data();
                    }

                    if (!streamState.headersFn(statusCode, headers)) {
                        streamState.cancel = true;
                        CFRunLoopStop(CFRunLoopGetCurrent());
                        break;
                    }
                }

                // Data callback
                if (numBytesRead > 0) {
                    if (!streamState.dataFn(static_cast<const unsigned char*>(&buf[0]), numBytesRead)) {
                        streamState.cancel = true;
                        CFRunLoopStop(CFRunLoopGetCurrent());
                        break;
                    }
                }

                break;
            case kCFStreamEventErrorOccurred:
                streamState.error = "Stream data error";
                CFRunLoopStop(CFRunLoopGetCurrent());
                break;
            case kCFStreamEventEndEncountered:
                CFRunLoopStop(CFRunLoopGetCurrent());
                break;
            }
        }, &streamContext)) {
            throw NetworkException("Failed to register stream handler", request.url);
        }

        // Register stream with runloop
        CFReadStreamScheduleWithRunLoop(requestStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

        // Open the stream, wait until complete
        if (!CFReadStreamOpen(requestStream)) {
            streamState.error = "Failed to open HTTP stream";
        } else {
            CFTimeInterval timeout = _timeout > 0 ? _timeout : 600;
            if (CFRunLoopRunInMode(kCFRunLoopDefaultMode, timeout, false) == kCFRunLoopRunTimedOut) {
                streamState.error = "Network time out";
            }
        }

        // Unschedule stream from runloop
        CFReadStreamUnscheduleFromRunLoop(requestStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

        // Handle errors
        if (!streamState.error.empty()) {
            throw NetworkException(streamState.error, request.url);
        }

        // Done
        CFReadStreamClose(requestStream);

        return !streamState.cancel;
    }

}
