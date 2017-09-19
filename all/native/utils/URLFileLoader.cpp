#include "URLFileLoader.h"
#include "core/BinaryData.h"
#include "utils/AssetUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

namespace carto {
    
    URLFileLoader::URLFileLoader(const std::string& tag, bool cacheFiles) :
        _tag(tag), _cacheFiles(cacheFiles)
    {
    }

    URLFileLoader::~URLFileLoader() {
    }

    bool URLFileLoader::loadFile(const std::string& url, std::shared_ptr<BinaryData>& data) const {
        // Check cache first
        {
            auto it = _cachedFiles.find(url);
            if (it != _cachedFiles.end()) {
                data = it->second;
                return true;
            }
        }
        
        // Check if http:// or https:// protocol is used
        if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://") {
            Log::Infof("%s: Loading from network: %s", _tag.c_str(), url.c_str());
            if (!NetworkUtils::GetHTTP(url, data, true)) {
                Log::Errorf("%s: Failed to load %s", _tag.c_str(), url.c_str());
            } else {
                if (_cacheFiles) {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _cachedFiles[url] = data;
                }
            }
            return true;
        }
        
        // Use synchronous loading for assets://
        if (url.substr(0, 9) == "assets://") {
            Log::Infof("%s: Loading from assets: %s", _tag.c_str(), url.c_str());
            data = AssetUtils::LoadAsset(url.substr(9));
            if (!data) {
                Log::Errorf("%s: Failed to load %s", _tag.c_str(), url.c_str());
            }
            return true;
        }
        
        // Other scheme, requires internal handling
        return false;
    }

    bool URLFileLoader::streamFile(const std::string& url, HandlerFunc handlerFn) const {
        // Check if http:// or https:// protocol is used
        if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://") {
            Log::Infof("%s: Loading from network: %s", _tag.c_str(), url.c_str());
            std::map<std::string, std::string> requestHeaders;
            std::map<std::string, std::string> responseHeaders;
            return NetworkUtils::GetHTTP(url, requestHeaders, responseHeaders, [&](std::uint64_t offset, std::uint64_t length, const unsigned char* buf, std::size_t size) -> bool {
                return handlerFn(length, buf, size);
            }, 0, Log::IsShowDebug());
        }
        
        // Use synchronous loading for assets://
        if (url.substr(0, 9) == "assets://") {
            Log::Infof("%s: Loading from assets: %s", _tag.c_str(), url.c_str());
            std::shared_ptr<BinaryData> data = AssetUtils::LoadAsset(url.substr(9)); // TODO: stream asset
            if (data) {
                return handlerFn(data->size(), data->data(), data->size());
            }
            else {
                Log::Errorf("%s: Failed to load %s", _tag.c_str(), url.c_str());
            }
            return false;
        }
    }
    
}
