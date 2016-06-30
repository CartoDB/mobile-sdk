/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NETWORKUTILS_H_
#define _CARTO_NETWORKUTILS_H_

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cstdint>

namespace carto {
    class BinaryData;

    class NetworkUtils {
    public:
        typedef std::function<bool(std::uint64_t, std::uint64_t, const unsigned char*, std::size_t)> HandlerFn;

        static bool GetHTTP(const std::string& url, std::shared_ptr<BinaryData>& responseData, bool log);

        static bool GetHTTP(const std::string& url, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, std::shared_ptr<BinaryData>& responseData, bool log);

        static int GetHTTP(const std::string& url, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, HandlerFn handler, std::uint64_t offset, bool log);

        static int GetMaxAgeHTTPHeader(const std::map<std::string, std::string>& headers);

        static std::string URLEncode(const std::string& value);

        static std::string URLEncodeMap(const std::multimap<std::string, std::string>& valueMap);

        static std::string URLDecode(const std::string& encValue);

        static std::multimap<std::string, std::string> URLDecodeMap(const std::string& encValueMap);

        static std::string BuildURLFromParameters(const std::string& baseURL, const std::map<std::string, std::string>& params);

        static std::string BuildURLFromParameters(const std::string& baseURL, const std::multimap<std::string, std::string>& params);

        static std::string SetURLProtocol(const std::string& url, const std::string& proto);

        static std::string ParseURLHostName(std::string url);

        static int ParseURLPort(std::string url);

        static std::string ParseURLPath(std::string url);

    private:
        NetworkUtils();

        static const int BUFFER_SIZE = 4096;
    };
    
}

#endif
