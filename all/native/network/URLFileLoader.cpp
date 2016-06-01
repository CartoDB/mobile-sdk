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
    
}
