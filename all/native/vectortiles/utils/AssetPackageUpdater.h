/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ASSETPACKAGEUPDATER_H_
#define _CARTO_ASSETPACKAGEUPDATER_H_

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <picojson/picojson.h>

namespace carto {
    class BinaryData;
    class AssetPackage;
    class MemoryAssetPackage;

    class AssetPackageUpdater {
    public:
        virtual ~AssetPackageUpdater();

        virtual std::shared_ptr<MemoryAssetPackage> update(const std::shared_ptr<AssetPackage>& assetPackage) const = 0;

    protected:
        struct FileInfo {
            std::string md5hash;
            std::uint64_t size = 0;

            bool operator == (const FileInfo& fileInfo) const { return md5hash == fileInfo.md5hash && size == fileInfo.size; }
            bool operator != (const FileInfo& fileInfo) const { return !(*this == fileInfo); }

            void verify(const std::shared_ptr<BinaryData>& data) const;
        };

        virtual std::shared_ptr<BinaryData> downloadFile(const std::string& fileName) const = 0;

        std::map<std::string, std::shared_ptr<BinaryData> > updateFiles(const std::map<std::string, FileInfo>& files1, const std::map<std::string, FileInfo>& files2) const;

        static std::map<std::string, FileInfo> ReadFileInfo(const picojson::value& fileData);

        static picojson::value ExtractFileData(const std::shared_ptr<BinaryData>& data);
    };

}

#endif
