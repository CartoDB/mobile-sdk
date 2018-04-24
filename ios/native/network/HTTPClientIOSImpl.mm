#include "HTTPClientIOSImpl.h"
#include "components/Exceptions.h"
#include "utils/CFUniquePtr.h"
#include "utils/Log.h"

#include <chrono>
#include <limits>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#import <Foundation/Foundation.h>

@interface URLConnection : NSObject <NSURLConnectionDataDelegate, NSURLConnectionDelegate>

-(NSError*)sendSynchronousRequest:(NSURLRequest*)request didReceiveResponse:(BOOL(^)(NSURLResponse*))responseHandler didReceiveData:(BOOL(^)(NSData*))dataHandler;

@end

@interface URLConnection ()
@property(nonatomic, strong) NSURLConnection* connection;
@property(nonatomic, strong) NSCondition* condition;
@property(nonatomic, strong) NSError* error;
@property(nonatomic, strong) BOOL(^responseHandler)(NSURLResponse*);
@property(nonatomic, strong) BOOL(^dataHandler)(NSData*);
@property(nonatomic) BOOL connectionDidFinishLoading;
@end

@implementation URLConnection

-(NSError*)sendSynchronousRequest:(NSURLRequest*)request didReceiveResponse:(BOOL(^)(NSURLResponse*))responseHandler didReceiveData:(BOOL(^)(NSData*))dataHandler {
    self.responseHandler = responseHandler;
    self.dataHandler = dataHandler;
    self.condition = [[NSCondition alloc] init];
    self.error = nil;
    self.connectionDidFinishLoading = NO;
    self.connection = [[NSURLConnection alloc] initWithRequest:request delegate:self startImmediately:NO];
    [self.connection setDelegateQueue:[[NSOperationQueue alloc] init]];
    [self.connection start];
    [self waitForConnectionToFinishLoading];
    return self.error;
}

-(void)waitForConnectionToFinishLoading {
    [self.condition lock];
    while (!self.connectionDidFinishLoading) {
        [self.condition wait];
    }
    [self.condition unlock];
}

-(void)connection:(NSURLConnection*)connection didReceiveResponse:(NSURLResponse*)response {
    if (self.responseHandler(response)) {
        [self connectionDidFinishLoading:connection];
    }
}

-(void)connection:(NSURLConnection*)connection didReceiveData:(NSData*)data {
    if (self.dataHandler(data)) {
        [self connectionDidFinishLoading:connection];
    }
}

-(void)connection:(NSURLConnection*)connection didFailWithError:(NSError*)error {
    [self.condition lock];
    self.error = error;
    self.connectionDidFinishLoading = YES;
    [self.condition signal];
    [self.condition unlock];    
}

-(void)connectionDidFinishLoading:(NSURLConnection*)connection {
    [self.condition lock];
    self.connectionDidFinishLoading = YES;
    [self.condition signal];
    [self.condition unlock];
}

#ifdef _CARTO_IGNORE_SSL_CERTS
-(void)connection:(NSURLConnection*)connection willSendRequestForAuthenticationChallenge:(NSURLAuthenticationChallenge*)challenge {
    if ([[challenge protectionSpace] authenticationMethod] == NSURLAuthenticationMethodServerTrust) {
        [[challenge sender] useCredential:[NSURLCredential credentialForTrust:[[challenge protectionSpace] serverTrust]] forAuthenticationChallenge:challenge];
    } else {
        [[challenge sender] performDefaultHandlingForAuthenticationChallenge:challenge];
    }
}
#endif

@end

namespace carto {

    HTTPClient::IOSImpl::IOSImpl(bool log) :
        _log(log),
        _timeout(-1)
    {
    }

    void HTTPClient::IOSImpl::setTimeout(int milliseconds) {
        _timeout = milliseconds;
    }

    bool HTTPClient::IOSImpl::makeRequest(const HTTPClient::Request& request, HeadersFunc headersFn, DataFunc dataFn) const {
        NSString* url = [[NSString alloc] initWithUTF8String:request.url.c_str()];
        NSMutableURLRequest* httpRequest = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:url]];
        [httpRequest setHTTPShouldUsePipelining:YES];

        if (_timeout > 0) {
            [httpRequest setTimeoutInterval:_timeout / 1000.0];
        }

        NSString* method = [[NSString alloc] initWithUTF8String:request.method.c_str()];
        [httpRequest setHTTPMethod:method];

        // Set request headers
        for (auto it = request.headers.begin(); it != request.headers.end(); it++) {
            NSString* key = [[NSString alloc] initWithUTF8String:it->first.c_str()];
            NSString* val = [[NSString alloc] initWithUTF8String:it->second.c_str()];
            [httpRequest addValue:val forHTTPHeaderField:key];
        }

        // Set request body, if contentType defined
        if (!request.contentType.empty()) {
            NSData* body = [NSData dataWithBytes:request.body.data() length:request.body.size()];
            [httpRequest setHTTPBody:body];
        }

        // Response and data handlers
        __block BOOL cancel = false;
        BOOL(^handleResponse)(NSURLResponse*) = ^BOOL(NSURLResponse* response) {
            NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)response;
            int statusCode = static_cast<int>([httpResponse statusCode]);

            std::map<std::string, std::string> headers;
            for (NSString* key in [httpResponse allHeaderFields]) {
                NSString* val = [[httpResponse allHeaderFields] objectForKey:key];
                headers[std::string([key UTF8String])] = std::string([val UTF8String]);
            }

            if (!headersFn(statusCode, headers)) {
                cancel = YES;
            }
            return cancel;
        };
        BOOL(^handleData)(NSData*) = ^BOOL(NSData* data) {
            if (!dataFn(reinterpret_cast<const unsigned char*>([data bytes]), [data length])) {
                cancel = YES;
            }
            return cancel;
        };

        // Send the request
        URLConnection* connection = [[URLConnection alloc] init];
        NSError* error = [connection sendSynchronousRequest:httpRequest didReceiveResponse:handleResponse didReceiveData:handleData];
        if (error) {
            NSString* description = [error localizedDescription];
            throw NetworkException(std::string([description UTF8String]), request.url);
        }

        return cancel == NO;
    }

}
