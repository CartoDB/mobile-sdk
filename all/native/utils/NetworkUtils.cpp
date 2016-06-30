#include "NetworkUtils.h"
#include "core/BinaryData.h"
#include "network/HTTPClient.h"
#include "utils/GeneralUtils.h"
#include "utils/Log.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <cstdint>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace carto {
    
    bool NetworkUtils::GetHTTP(const std::string& url, std::shared_ptr<BinaryData>& responseData, bool log) {
        std::map<std::string, std::string> requestHeaders;
        std::map<std::string, std::string> responseHeaders;
        return GetHTTP(url, requestHeaders, responseHeaders, responseData, log);
    }

    bool NetworkUtils::GetHTTP(const std::string& url, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, std::shared_ptr<BinaryData>& responseData, bool log) {
        HTTPClient client(log);
        return client.get(url, requestHeaders, responseHeaders, responseData) == 0;
    }

    int NetworkUtils::GetHTTP(const std::string& url, const std::map<std::string, std::string>& requestHeaders, std::map<std::string, std::string>& responseHeaders, HandlerFn handler, std::uint64_t offset, bool log) {
        HTTPClient client(log);
        return client.get(url, requestHeaders, responseHeaders, handler, offset);
    }

    int NetworkUtils::GetMaxAgeHTTPHeader(const std::map<std::string, std::string>& headers) {
        for (auto it = headers.begin(); it != headers.end(); it++) {
            if (boost::iequals(it->first, "Cache-Control")) {
                std::vector<std::string> values = GeneralUtils::Split(it->second, ',');
                for (auto it2 = values.begin(); it2 != values.end(); it2++) {
                    std::string value = boost::trim_copy(*it2);
                    if (boost::iequals(value, "no-cache") || boost::iequals(value, "no-store")) {
                        return 0;
                    }
                    if (boost::iequals(value.substr(0, 8), "max-age=")) {
                        std::string maxAge = boost::trim_copy(value.substr(8));
                        std::string::size_type pos = maxAge.find(' ');
                        if (pos != std::string::npos) {
                            maxAge = maxAge.substr(0, pos);
                        }
                        try {
                            return boost::lexical_cast<int>(maxAge);
                        }
                        catch (const boost::bad_lexical_cast&) {
                            Log::Errorf("NetworkUtils::GetMaxAgeHTTPHeader: Invalid max-age value: %s", maxAge.c_str());
                            return -1;
                        }
                    }
                }
            }
        }
        return -1;
    }
    
    std::string NetworkUtils::URLEncode(const std::string& value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;
        for (auto it = value.begin(); it != value.end(); it++) {
            char c = *it;
            if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
                continue;
            }
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
        return escaped.str();
    }

    std::string NetworkUtils::URLEncodeMap(const std::multimap<std::string, std::string>& valueMap) {
        std::string encValueMap;
        for (auto it = valueMap.begin(); it != valueMap.end(); it++) {
            if (!encValueMap.empty()) {
                encValueMap += '&';
            }
            encValueMap += URLEncode(it->first);
            encValueMap += "=";
            encValueMap += URLEncode(it->second);
        }
        return encValueMap;
    }

    std::string NetworkUtils::URLDecode(const std::string& encValue) {
        std::string value;
        for (std::size_t i = 0; i < encValue.size(); i++) {
            char c = encValue[i];

            if (c == '%' && i + 2 < encValue.size()) {
                int n = 0;
                sscanf(encValue.substr(i + 1, 2).c_str(), "%x", &n);
                value += static_cast<char>(n);
                i += 2;
                continue;
            }

            if (c == '+') {
                value += ' ';
                continue;
            }

            value += c;
        }
        return value;
    }

    std::multimap<std::string, std::string> NetworkUtils::URLDecodeMap(const std::string& encValueMap) {
        std::multimap<std::string, std::string> valueMap;
        std::stringstream stream(encValueMap);
        std::string encKeyValue;
        while (std::getline(stream, encKeyValue, '&')) {
            std::string::size_type pos = encKeyValue.find('=');
            if (pos == std::string::npos) {
                valueMap.insert({ URLDecode(encKeyValue), std::string() });
            }
            else {
                valueMap.insert({ URLDecode(encKeyValue.substr(0, pos)), URLDecode(encKeyValue.substr(pos + 1)) });
            }
        }
        return valueMap;
    }

    std::string NetworkUtils::BuildURLFromParameters(const std::string& baseURL, const std::map<std::string, std::string>& params) {
        return BuildURLFromParameters(baseURL, std::multimap<std::string, std::string>(params.begin(), params.end()));
    }

    std::string NetworkUtils::BuildURLFromParameters(const std::string& baseURL, const std::multimap<std::string, std::string>& params) {
        std::string url = baseURL;
        if (!params.empty()) {
            url += (url.find('?') == std::string::npos ? '?' : '&');
            url += URLEncodeMap(params);
        }
        return url;
    }

    std::string NetworkUtils::SetURLProtocol(const std::string& url, const std::string& proto) {
        std::string::size_type pos = url.find("://");
        if (pos == std::string::npos) {
            Log::Errorf("NetworkUtils::SetURLProtocol: Could not detect protocol");
            return url;
        }
        return proto + url.substr(pos);
    }

    std::string NetworkUtils::ParseURLHostName(std::string url) {
        // Remove http://, if exists
        std::size_t index = url.find("://");
        if (index != std::string::npos) {
            url.erase(0, index + 3);
        }

        // Remove everything after first /, if exists
        index = url.find("/");
        if (index != std::string::npos) {
            url.erase(url.begin() + index, url.end());
        }

        // Remove port
        index = url.find(":");
        if (index != std::string::npos) {
            url.erase(url.begin() + index, url.end());
        }

        return url;
    }

    int NetworkUtils::ParseURLPort(std::string url) {
        // Remove http://, if exists
        std::size_t index = url.find("://");
        if (index != std::string::npos) {
            url.erase(0, index + 3);
        }

        // Remove everything before :, if exists
        index = url.find(":");
        if (index != std::string::npos) {
            url.erase(0, index + 1);
        }
        else {
            return 80;
        }

        // Remove everything after first /, if exists
        index = url.find("/");
        if (index != std::string::npos) {
            url.erase(url.begin() + index, url.end());
        }

        return std::atoi(url.c_str());
    }

    std::string NetworkUtils::ParseURLPath(std::string url) {
        // Remove http://, if exists
        std::size_t index = url.find("://");
        if (index != std::string::npos) {
            url.erase(0, index + 3);
        }

        // Remove everything before first /, if exists
        index = url.find("/");
        if (index != std::string::npos) {
            url.erase(0, index);
        }

        return url;
    }

    NetworkUtils::NetworkUtils() {
    }

}
