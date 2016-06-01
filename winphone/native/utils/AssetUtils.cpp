#include "AssetUtils.h"
#include "core/BinaryData.h"
#include "utils/Log.h"

#include <stdio.h>

#include <utf8.h>

namespace carto {

    std::shared_ptr<BinaryData> AssetUtils::LoadAsset(const std::string& path) {
        std::wstring wpath;
        utf8::utf8to16(path.begin(), path.end(), std::back_inserter(wpath));
        Platform::String^ appPath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
        Platform::String^ fullPath = appPath + L"\\" + ref new Platform::String(wpath.c_str());
        std::wstring wfullPath = fullPath->Data();
        FILE* fpRaw = _wfopen(wfullPath.c_str(), L"rb");
        if (!fpRaw) {
            fullPath = appPath + L"\\Assets\\" + ref new Platform::String(wpath.c_str());
            wfullPath = fullPath->Data();
            fpRaw = _wfopen(wfullPath.c_str(), L"rb");
        }
        if (fpRaw) {
            std::shared_ptr<FILE> fp(fpRaw, fclose);
            fseek(fp.get(), 0, SEEK_END);
            long size = ftell(fp.get());
            if (size < 0) {
                Log::Errorf("AssetManager::LoadAsset: Error detecting asset size: %s", path.c_str());
                return std::shared_ptr<BinaryData>();
            }
            std::vector<unsigned char> data(size);
            fseek(fp.get(), 0, SEEK_SET);
            fread(data.data(), 1, size, fp.get());
            return std::make_shared<BinaryData>(std::move(data));
        } else {
            Log::Errorf("AssetUtils::LoadAsset: Asset not found: %s", path.c_str());
            return std::shared_ptr<BinaryData>();
        }
    }

    AssetUtils::AssetUtils() {
    }

}
