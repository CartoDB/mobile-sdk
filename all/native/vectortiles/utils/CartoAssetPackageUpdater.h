/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOASSETPACKAGEUPDATER_H_
#define _CARTO_CARTOASSETPACKAGEUPDATER_H_

#include "vectortiles/utils/AssetPackageUpdater.h"

namespace carto {

    class CartoAssetPackageUpdater : public AssetPackageUpdater {
    public:
        CartoAssetPackageUpdater(const std::string& schema, const std::string& styleName);
        virtual ~CartoAssetPackageUpdater();

        virtual std::shared_ptr<MemoryAssetPackage> update(const std::shared_ptr<AssetPackage>& assetPackage) const;

    protected:
        virtual std::shared_ptr<BinaryData> downloadFile(const std::string& fileName) const;

        static const std::string STYLE_SERVICE_URL;

        const std::string _schema;
        const std::string _styleName;
    };

}

#endif