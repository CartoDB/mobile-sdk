#include "CartoSQLService.h"
#include "core/BinaryData.h"
#include "geometry/FeatureCollection.h"
#include "geometry/GeoJSONGeometryReader.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "network/HTTPClient.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

namespace carto {

    CartoSQLService::CartoSQLService() :
        _username(),
        _apiTemplate(DEFAULT_API_TEMPLATE),
        _mutex()
    {
    }

    CartoSQLService::~CartoSQLService() {
    }

    std::string CartoSQLService::getUsername() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _username;
    }
        
    void CartoSQLService::setUsername(const std::string& username) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _username = username;
    }

    std::string CartoSQLService::getAPITemplate() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _apiTemplate;
    }

    void CartoSQLService::setAPITemplate(const std::string& apiTemplate) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _apiTemplate = apiTemplate;
    }

    Variant CartoSQLService::queryData(const std::string& sql) const {
        std::map<std::string, std::string> urlParams;
        urlParams["q"] = sql;
        std::string result = executeQuery(urlParams);

        // Parse result
        return Variant::FromString(result);
    }

    std::shared_ptr<FeatureCollection> CartoSQLService::queryFeatures(const std::string& sql, const std::shared_ptr<Projection>& proj) const {
        std::map<std::string, std::string> urlParams;
        urlParams["q"] = sql;
        urlParams["format"] = "GeoJSON";
        std::string result = executeQuery(urlParams);

        // Parse result
        GeoJSONGeometryReader reader;
        reader.setTargetProjection(proj);
        return reader.readFeatureCollection(result);
    }

    std::string CartoSQLService::executeQuery(const std::map<std::string, std::string>& urlParams) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // Build URL
        std::map<std::string, std::string> tagValues = { { "user", _username },{ "username", _username } };
        std::string baseURL = GeneralUtils::ReplaceTags(_apiTemplate, tagValues, "{", "}", false) + "/api/v2/sql";
        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, urlParams);

        // Perform HTTP request
        HTTPClient client(Log::IsShowDebug());
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.get(url, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            std::string result;
            if (responseData) {
                result = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
            }
            throw GenericException("Failed to execute query", result);
        }

        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        return result;
    }

    const std::string CartoSQLService::DEFAULT_API_TEMPLATE = "http://{user}.cartodb.com";

}
