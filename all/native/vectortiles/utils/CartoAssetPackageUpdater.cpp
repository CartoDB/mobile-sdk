#include "CartoAssetPackageUpdater.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/MemoryAssetPackage.h"
#include "utils/Log.h"

#include <utility>

#include <picojson/picojson.h>

namespace carto {

    CartoAssetPackageUpdater::CartoAssetPackageUpdater(const std::string& schema, const std::string& styleName) :
        _schema(schema),
        _styleName(styleName)
    {
    }

    CartoAssetPackageUpdater::~CartoAssetPackageUpdater() {
    }

    std::shared_ptr<MemoryAssetPackage> CartoAssetPackageUpdater::update(const std::shared_ptr<AssetPackage>& assetPackage) const {
        std::string projectFileName = _styleName + ".json";

        std::shared_ptr<BinaryData> fileData1 = assetPackage->loadAsset(projectFileName);
        picojson::value projectJson1 = ExtractFileData(fileData1);
        std::map<std::string, FileInfo> files1 = ReadFileInfo(projectJson1);

        std::shared_ptr<BinaryData> fileData2 = downloadFile(projectFileName);
        picojson::value projectJson2 = ExtractFileData(fileData2);
        std::map<std::string, FileInfo> files2 = ReadFileInfo(projectJson2);

        std::map<std::string, std::shared_ptr<BinaryData> > updatedAssets = updateFiles(files1, files2);
        if (projectJson1 != projectJson2) {
            updatedAssets[projectFileName] = fileData2;
        }
        return std::make_shared<MemoryAssetPackage>(updatedAssets, assetPackage);
    }

    std::shared_ptr<BinaryData> CartoAssetPackageUpdater::downloadFile(const std::string& fileName) const {
        std::vector<std::string> pathParts = GeneralUtils::Split(_schema + "/" + fileName, '/');
        std::transform(pathParts.begin(), pathParts.end(), pathParts.begin(), NetworkUtils::URLEncode);
        std::string urlEncodedFileName = GeneralUtils::Join(pathParts, '/');
        std::string url = STYLE_SERVICE_URL + urlEncodedFileName;

        std::shared_ptr<BinaryData> responseData;
        if (!NetworkUtils::GetHTTP(url, responseData, Log::IsShowDebug())) {
            Log::Warnf("CartoAssetPackageUpdater: Failed to fetch tile style data");
        }
        return responseData;
    }

    const std::string CartoAssetPackageUpdater::STYLE_SERVICE_URL = "http://mobile-api.carto.com/styles/";

}
