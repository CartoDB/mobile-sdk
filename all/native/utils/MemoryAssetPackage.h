/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MEMORYASSETPACKAGE_H_
#define _CARTO_MEMORYASSETPACKAGE_H_

#include "utils/AssetPackage.h"

#include <map>
#include <mutex>

namespace carto {

    class MemoryAssetPackage : public AssetPackage {
    public:
        explicit MemoryAssetPackage(const std::map<std::string, std::shared_ptr<BinaryData> >& localAssets);
        MemoryAssetPackage(const std::map<std::string, std::shared_ptr<BinaryData> >& localAssets, const std::shared_ptr<AssetPackage>& baseAssetPackage);
        virtual ~MemoryAssetPackage();

        std::vector<std::string> getLocalAssetNames() const;

        virtual std::vector<std::string> getAssetNames() const;

        virtual std::shared_ptr<BinaryData> loadAsset(const std::string& name) const;

    private:
        const std::map<std::string, std::shared_ptr<BinaryData> > _localAssets;

        const std::shared_ptr<AssetPackage> _baseAssetPackage;
    };

}

#endif
