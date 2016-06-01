#include "ZippedAssetPackage.h"
#include "core/BinaryData.h"
#include "utils/Log.h"

#define MINIZ_HEADER_FILE_ONLY
#include <miniz.c>

namespace carto {

    ZippedAssetPackage::ZippedAssetPackage(const std::shared_ptr<BinaryData>& zipData) :
        _handle(),
        _zipData(zipData),
        _assetIndexMap()
    {
        if (!_zipData) {
            Log::Error("ZippedAssetPackage: Empty data");
            return;
        }
    
        _handle = std::make_shared<mz_zip_archive>();
        mz_zip_archive* zip = static_cast<mz_zip_archive*>(_handle.get());
        memset(zip, 0, sizeof(mz_zip_archive));
        std::shared_ptr<std::vector<unsigned char> > data = _zipData->getDataPtr();
        if (!mz_zip_reader_init_mem(zip, data->data(), data->size(), 0)) {
            Log::Error("ZippedAssetPackage: Could not open archive");
            return;
        }
    
        for (unsigned int i = 0; i < mz_zip_reader_get_num_files(zip); i++) {
            mz_zip_archive_file_stat stat;
            if (!mz_zip_reader_file_stat(zip, i, &stat)) {
                Log::Error("ZippedAssetPackage: Could not read file stats");
                continue;
            }
    
            _assetIndexMap[stat.m_filename] = i;
        }
    }
    
    ZippedAssetPackage::~ZippedAssetPackage() {
        mz_zip_archive* zip = static_cast<mz_zip_archive*>(_handle.get());
        if (zip) {
            mz_zip_reader_end(zip);
        }
    }
    
    std::vector<std::string> ZippedAssetPackage::getAssetNames() const {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<std::string> names;
        names.reserve(_assetIndexMap.size());
        for (auto it = _assetIndexMap.begin(); it != _assetIndexMap.end(); it++) {
            names.push_back(it->first);
        }
        return names;
    }
    
    std::shared_ptr<BinaryData> ZippedAssetPackage::loadAsset(const std::string& name) const {
        std::lock_guard<std::mutex> lock(_mutex);

        mz_zip_archive* zip = static_cast<mz_zip_archive*>(_handle.get());
        if (!zip) {
            return std::shared_ptr<BinaryData>();
        }
    
        auto it = _assetIndexMap.find(name);
        if (it == _assetIndexMap.end()) {
            return std::shared_ptr<BinaryData>();
        }
        std::size_t elementSize = 0;
        std::shared_ptr<unsigned char> elementData(static_cast<unsigned char*>(mz_zip_reader_extract_to_heap(zip, it->second, &elementSize, 0)), mz_free);
        if (!elementData) {
            Log::Error("ZippedAssetPackage::loadAsset: Could not load archive asset");
            return std::shared_ptr<BinaryData>();
        }
        return std::make_shared<BinaryData>(elementData.get(), elementSize);
    }
    
}
