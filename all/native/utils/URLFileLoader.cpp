#include "URLFileLoader.h"
#include "core/BinaryData.h"
#include "utils/AssetUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <stdext/utf8_filesystem.h>

namespace carto {
    
    URLFileLoader::URLFileLoader() :
        _caching(false),
        _localFiles(false),
        _cachedFiles(),
        _mutex()
    {
    }

    URLFileLoader::~URLFileLoader() {
    }

    void URLFileLoader::setCaching(bool caching) {
        std::lock_guard<std::mutex> lock(_mutex);
        _caching = caching;
    }

    void URLFileLoader::setLocalFiles(bool localFiles) {
        std::lock_guard<std::mutex> lock(_mutex);
        _localFiles = localFiles;
    }

    bool URLFileLoader::isSupported(const std::string& url) const {
        if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://") {
            return true;
        }
        if (url.substr(0, 9) == "assets://") {
            return true;
        }
        if (url.substr(0, 7) == "file://") {
            std::lock_guard<std::mutex> lock(_mutex);
            return _localFiles;
        }
        return false;
    }

    bool URLFileLoader::load(const std::string& url, std::shared_ptr<BinaryData>& data) const {
        // Check cache first
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _cachedFiles.find(url);
            if (it != _cachedFiles.end()) {
                data = it->second;
                return true;
            }
        }
        
        // Stream and store in cache
        std::vector<unsigned char> contents;
        if (stream(url, [&](std::uint64_t length, const unsigned char* buf, std::size_t size) {
            contents.insert(contents.end(), buf, buf + size);
            return true;
        })) {
            data = std::make_shared<BinaryData>(std::move(contents));

            std::lock_guard<std::mutex> lock(_mutex);
            if (_caching) {
                _cachedFiles[url] = data;
            }
            return true;
        }
        return false;
    }

    bool URLFileLoader::stream(const std::string& url, HandlerFunc handlerFn) const {
        // Check if http:// or https:// protocol is used
        if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://") {
            Log::Debugf("URLFileLoader: Streaming from network: %s", url.c_str());
            std::map<std::string, std::string> requestHeaders;
            std::map<std::string, std::string> responseHeaders;
            return NetworkUtils::StreamHTTPResponse("GET", url, requestHeaders, responseHeaders, [&](std::uint64_t offset, std::uint64_t length, const unsigned char* buf, std::size_t size) -> bool {
                return handlerFn(length, buf, size);
            }, 0, Log::IsShowDebug()) == 0;
        }
        
        // Use synchronous loading for assets://
        if (url.substr(0, 9) == "assets://") {
            Log::Debugf("URLFileLoader: Streaming asset: %s", url.c_str());
            std::shared_ptr<BinaryData> data = AssetUtils::LoadAsset(url.substr(9)); // TODO: stream asset
            if (!data) {
                Log::Errorf("URLFileLoader: Failed to load %s", url.c_str());
                return false;
            }
            return handlerFn(data->size(), data->data(), data->size());
        }

        // Local files? Only if explicitly enabled due to security reasons
        if (url.substr(0, 7) == "file://") {
            if (_localFiles) {
                Log::Debugf("URLFileLoader: Streaming local file: %s", url.c_str());
                FILE* fpRaw = utf8_filesystem::fopen(url.substr(7).c_str(), "rb");
                if (!fpRaw) {
                    Log::Errorf("URLFileLoader: Failed to load %s", url.c_str());
                    return false;
                } else {
                    std::shared_ptr<FILE> fp(fpRaw, fclose);
                    utf8_filesystem::fseek64(fp.get(), 0, SEEK_END);
                    std::uint64_t length = utf8_filesystem::ftell64(fp.get());
                    utf8_filesystem::fseek64(fp.get(), 0, SEEK_SET);
                    while (!feof(fp.get())) {
                        unsigned char buf[4096];
                        std::size_t n = fread(buf, sizeof(unsigned char), sizeof(buf) / sizeof(unsigned char), fp.get());
                        if (!handlerFn(length, buf, n)) {
                            return false;
                        }
                    }
                }
                return true;
            } else {
                Log::Errorf("URLFileLoader: Trying to access local files without perminssions: %s", url.c_str());
                return false;
            }
        }

        // Unsupported schema
        Log::Errorf("URLFileLoader: Unsupported URL schema %s", url.c_str());
        return false;
    }
    
}
