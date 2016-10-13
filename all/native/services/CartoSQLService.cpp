#include "CartoSQLService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
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
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        // Build URL
        std::map<std::string, std::string> tagValues = { { "user", _username },{ "username", _username } };
        std::string url = GeneralUtils::ReplaceTags(_apiTemplate, tagValues, "{", "}", false) + "/api/v2/sql";

        std::map<std::string, std::string> urlParams;
        urlParams["q"] = sql;
        url = NetworkUtils::BuildURLFromParameters(url, urlParams);

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

        // Parse result
        std::string result(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        return Variant::FromString(result);
    }

    const std::string CartoSQLService::DEFAULT_API_TEMPLATE = "http://{user}.cartodb.com";

}
