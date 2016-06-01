#include "CartoSQLService.h"
#include "core/BinaryData.h"
#include "network/HTTPClient.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

namespace carto {

    CartoSQLService::CartoSQLService() :
        _username(),
        _apiKey(),
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

    std::string CartoSQLService::getAPIKey() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _apiKey;
    }

    void CartoSQLService::setAPIKey(const std::string& apiKey) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _apiKey = apiKey;
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
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // Build URL
        std::string url = _apiTemplate;
        url += "/api/v2/sql";

        std::map<std::string, std::string> urlParams;
        urlParams["q"] = sql;
        if (!_apiKey.empty()) {
            urlParams["api_key"] = _apiKey;
            url = NetworkUtils::SetURLProtocol(url, "https");
        }
        url = NetworkUtils::BuildURLFromParameters(url, urlParams);

        // Perform HTTP request
        HTTPClient client(false);
        std::shared_ptr<BinaryData> responseData;
        std::map<std::string, std::string> responseHeaders;
        if (client.get(url, std::map<std::string, std::string>(), responseHeaders, responseData) != 0) {
            // TODO: expect error?
            Log::Error("CartoSQLService::queryData: Failed to read response");
            return Variant();
        }

        // Parse result
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        return Variant::FromString(result);
    }

    const std::string CartoSQLService::DEFAULT_API_TEMPLATE = "http://{user}.cartodb.com";

}
