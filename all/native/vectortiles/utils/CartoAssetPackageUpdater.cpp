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

        std::shared_ptr<BinaryData> projectData1 = assetPackage->loadAsset(projectFileName);
        picojson::value filesJson1 = ExtractFileData(projectData1);
        std::map<std::string, FileInfo> files1 = ReadFileInfo(filesJson1);

        std::shared_ptr<BinaryData> projectData2 = downloadFile(projectFileName);
        picojson::value filesJson2 = ExtractFileData(projectData2);
        std::map<std::string, FileInfo> files2 = ReadFileInfo(filesJson2);

        std::map<std::string, std::shared_ptr<BinaryData> > updatedAssets = updateFiles(files1, files2);
        if (projectData1 && projectData2) {
            if (*projectData1 != *projectData2) {
                updatedAssets[projectFileName] = projectData2;
            }
        }
        return std::make_shared<MemoryAssetPackage>(updatedAssets, assetPackage);
    }

    std::shared_ptr<BinaryData> CartoAssetPackageUpdater::downloadFile(const std::string& fileName) const {
        std::vector<std::string> pathParts = GeneralUtils::Split(_schema + "/" + fileName, '/');
        std::transform(pathParts.begin(), pathParts.end(), pathParts.begin(), NetworkUtils::URLEncode);
        std::string urlEncodedFileName = GeneralUtils::Join(pathParts, '/');
        std::string url = STYLE_SERVICE_URL + urlEncodedFileName;

        std::map<std::string, std::string> requestHeaders = NetworkUtils::CreateAppRefererHeader();
        std::map<std::string, std::string> responseHeaders;
        std::shared_ptr<BinaryData> responseData;
        if (!NetworkUtils::GetHTTP(url, requestHeaders, responseHeaders, responseData, Log::IsShowDebug())) {
            throw NetworkException("Failed to fetch tile style data");
        }
        return responseData;
    }

    const std::string CartoAssetPackageUpdater::STYLE_SERVICE_URL = "https://api.nutiteq.com/styles/";

}
