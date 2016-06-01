#include "FileUtils.h"
#include "utils/AssetUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <vector>

namespace carto {
    
    std::string FileUtils::GetFileName(const std::string& fullPath) {
        std::string::size_type pos = fullPath.find_last_of("/");
        if (pos == std::string::npos) {
            return fullPath;
        }
        return fullPath.substr(pos + 1);
    }

    std::string FileUtils::GetFilePath(const std::string& fullPath) {
        std::string::size_type pos = fullPath.find_last_of("/");
        if (pos == std::string::npos) {
            return std::string();
        }
        return fullPath.substr(0, pos + 1);
    }

    std::string FileUtils::NormalizePath(const std::string& path) {
        std::vector<std::string> components;
        std::string::size_type start = 0, end = 0;
        while ((end = path.find('/', start)) != std::string::npos) {
            if (components.empty() || end != start) {
                components.push_back(path.substr(start, end - start));
            }
            start = end + 1;
        }
        components.push_back(path.substr(start));
        
        for (std::size_t i = 0; i < components.size(); ) {
            if (components[i] == ".") {
                components.erase(components.begin() + i);
            } else if (components[i] == "..") {
                if (i > 0) {
                    --i;
                    components.erase(components.begin() + i);
                }
                components.erase(components.begin() + i);
            } else {
                i++;
            }
        }
        
        std::string normalizedPath;
        for (std::size_t i = 0; i < components.size(); i++) {
            normalizedPath.append(i > 0 ? "/" : "").append(components[i]);
        }
        return normalizedPath;
    }
    
    FileUtils::FileUtils() {
    }

}
