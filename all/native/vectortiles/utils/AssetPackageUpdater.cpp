#include "AssetPackageUpdater.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "utils/MemoryAssetPackage.h"
#include "utils/Log.h"

#include <utility>

#include <picojson/picojson.h>

#include <botan/botan_all.h>

namespace {

    std::string CalculateMD5Hash(const std::shared_ptr<carto::BinaryData>& data) {
        if (!data) {
            return std::string();
        }

        std::unique_ptr<Botan::HashFunction> hash(new Botan::MD5);
        hash->update(reinterpret_cast<const std::uint8_t*>(data->data()), data->size());
        return Botan::hex_encode(hash->final(), false);
    }

}

namespace carto {

    AssetPackageUpdater::~AssetPackageUpdater() {
    }

    void AssetPackageUpdater::FileInfo::verify(const std::shared_ptr<BinaryData>& data) const {
        if (!data) {
            throw NullArgumentException("Null data");
        }
        if (data->size() != size) {
            throw GenericException("Sizes do not match");
        }
        if (CalculateMD5Hash(data) != md5hash) {
            throw GenericException("MD5 hashes do not match");
        }
    }

    std::map<std::string, std::shared_ptr<BinaryData> > AssetPackageUpdater::updateFiles(const std::map<std::string, FileInfo>& files1, const std::map<std::string, FileInfo>& files2) const {
        std::map<std::string, std::shared_ptr<BinaryData> > updatedAssets;
        for (auto it2 = files2.begin(); it2 != files2.end(); it2++) {
            auto it1 = files1.find(it2->first);
            if (it1 != files1.end()) {
                if (it1->second == it2->second) {
                    continue;
                }
            }

            std::shared_ptr<BinaryData> data = downloadFile(it2->first);
            try {
                it2->second.verify(data);
            }
            catch (const std::exception& ex) {
                throw FileException(std::string("Failed to update file, verification failed: ") + ex.what(), it2->first);
            }
            updatedAssets[it2->first] = data;
        }

        for (auto it1 = files1.begin(); it1 != files1.end(); it1++) {
            if (files2.find(it1->first) == files2.end()) {
                updatedAssets[it1->first] = std::shared_ptr<BinaryData>();
            }
        }

        return updatedAssets;
    }

    std::map<std::string, AssetPackageUpdater::FileInfo> AssetPackageUpdater::ReadFileInfo(const picojson::value& fileData) {
        std::map<std::string, FileInfo> files;
        if (fileData.is<picojson::object>()) {
            for (const std::pair<const std::string, picojson::value>& file : fileData.get<picojson::object>()) {
                FileInfo fileInfo;
                fileInfo.md5hash = file.second.get("md5").get<std::string>();
                fileInfo.size = file.second.get("size").get<std::int64_t>();
                files[file.first] = fileInfo;
            }
        }
        return files;
    }

    picojson::value AssetPackageUpdater::ExtractFileData(const std::shared_ptr<BinaryData>& data) {
        if (!data) {
            throw NullArgumentException("No data");
        }

        std::string json(reinterpret_cast<const char*>(data->data()), data->size());
        picojson::value projectInfo;
        std::string err = picojson::parse(projectInfo, json);
        if (!err.empty()) {
            throw ParseException(std::string("Failed to parse style info: ") + err, json);
        }
        return projectInfo.get("files");
    }

}
