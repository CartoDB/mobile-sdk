#include "AssetUtils.h"
#include "core/BinaryData.h"
#include <fstream>
#include <iterator>
#include <vector>

namespace carto {
    std::shared_ptr<BinaryData> AssetUtils::LoadAsset(const std::string& path) {
        std::ifstream input( "/assets/"+path, std::ios::binary );
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
        return std::make_shared<BinaryData>(std::move(buffer));
    }

    AssetUtils::AssetUtils() {
    }

    std::mutex AssetUtils::_Mutex;
}
