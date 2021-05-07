#include "ZippedAssetPackage.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <miniz.h>

#include <string.h>

namespace carto {

    ZippedAssetPackage::ZippedAssetPackage(const std::shared_ptr<BinaryData>& zipData) :
        _zipData(zipData),
        _baseAssetPackage(),
        _handle(),
        _assetIndexMap()
    {
        initialize();
    }
    
    ZippedAssetPackage::ZippedAssetPackage(const std::shared_ptr<BinaryData>& zipData, const std::shared_ptr<AssetPackage>& baseAssetPackage) :
        _zipData(zipData),
        _baseAssetPackage(baseAssetPackage),
        _handle(),
        _assetIndexMap()
    {
        initialize();
    }
    
    ZippedAssetPackage::~ZippedAssetPackage() {
        deinitialize();
    }
    
    std::vector<std::string> ZippedAssetPackage::getLocalAssetNames() const {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<std::string> names;
        names.reserve(_assetIndexMap.size());
        for (auto it = _assetIndexMap.begin(); it != _assetIndexMap.end(); it++) {
            if (std::find(names.begin(), names.end(), it->first) == names.end()) {
                names.push_back(it->first);
            }
        }
        return names;
    }
    
    std::vector<std::string> ZippedAssetPackage::getAssetNames() const {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<std::string> names;
        if (_baseAssetPackage) {
            names = _baseAssetPackage->getAssetNames();
        }
        names.reserve(names.size() + _assetIndexMap.size());
        for (auto it = _assetIndexMap.begin(); it != _assetIndexMap.end(); it++) {
            if (std::find(names.begin(), names.end(), it->first) == names.end()) {
                names.push_back(it->first);
            }
        }
        return names;
    }
    
    std::shared_ptr<BinaryData> ZippedAssetPackage::loadAsset(const std::string& name) const {
        std::lock_guard<std::mutex> lock(_mutex);

        auto it = _assetIndexMap.find(name);
        if (it == _assetIndexMap.end()) {
            if (_baseAssetPackage) {
                return _baseAssetPackage->loadAsset(name);
            }
            return std::shared_ptr<BinaryData>();
        }

        mz_zip_archive* zip = static_cast<mz_zip_archive*>(_handle.get());
        if (!zip) {
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

    void ZippedAssetPackage::initialize() {
        if (!_zipData) {
            throw NullArgumentException("Null zipData");
        }
    
        _handle = std::make_shared<mz_zip_archive>();
        mz_zip_archive* zip = static_cast<mz_zip_archive*>(_handle.get());
        memset(zip, 0, sizeof(mz_zip_archive));
        std::shared_ptr<std::vector<unsigned char> > data = _zipData->getDataPtr();
        if (!mz_zip_reader_init_mem(zip, data->data(), data->size(), 0)) {
            throw GenericException("Could not open ZIP archive");
        }
    
        for (unsigned int i = 0; i < mz_zip_reader_get_num_files(zip); i++) {
            mz_zip_archive_file_stat stat;
            if (!mz_zip_reader_file_stat(zip, i, &stat)) {
                throw GenericException("Could not read ZIP archive file stats");
            }
    
            _assetIndexMap[stat.m_filename] = i;
        }
    }

    void ZippedAssetPackage::deinitialize() {
        mz_zip_archive* zip = static_cast<mz_zip_archive*>(_handle.get());
        if (zip) {
            mz_zip_reader_end(zip);
        }
        _handle.reset();
    }
    
}
