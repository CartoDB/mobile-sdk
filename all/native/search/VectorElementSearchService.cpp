#ifdef _CARTO_SEARCH_SUPPORT

#include "VectorElementSearchService.h"
#include "core/MapEnvelope.h"
#include "components/Exceptions.h"
#include "datasources/VectorDataSource.h"
#include "graphics/ViewState.h"
#include "renderers/components/CullState.h"
#include "geometry/Geometry.h"
#include "search/utils/SearchProxy.h"
#include "projections/Projection.h"
#include "vectorelements/VectorElement.h"
#include "utils/Log.h"

namespace carto {

    VectorElementSearchService::VectorElementSearchService(const std::shared_ptr<VectorDataSource>& dataSource) :
        _dataSource(dataSource),
        _maxResults(1000),
        _mutex()
    {
        if (!dataSource) {
            throw NullArgumentException("Null dataSource");
        }
    }

    VectorElementSearchService::~VectorElementSearchService() {
    }

    const std::shared_ptr<VectorDataSource>& VectorElementSearchService::getDataSource() const {
        return _dataSource;
    }

    int VectorElementSearchService::getMaxResults() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _maxResults;
    }

    void VectorElementSearchService::setMaxResults(int maxResults) {
        std::lock_guard<std::mutex> lock(_mutex);
        _maxResults = maxResults;
    }

    std::vector<std::shared_ptr<VectorElement> > VectorElementSearchService::findElements(const std::shared_ptr<SearchRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        SearchProxy proxy(request, _dataSource->getDataExtent(), _dataSource->getProjection());
        MapBounds searchBounds = proxy.getSearchBounds();
        auto cullState = std::make_shared<CullState>(MapEnvelope(searchBounds), ViewState());
        int maxResults = 0;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            maxResults = _maxResults;
        }

        std::vector<std::shared_ptr<VectorElement> > elements;
        if (std::shared_ptr<VectorData> vectorData = _dataSource->loadElements(cullState)) {
            for (std::size_t i = 0; i < vectorData->getElements().size(); i++) {
                if (static_cast<int>(elements.size()) >= maxResults) {
                    break;
                }

                const std::shared_ptr<VectorElement>& element = vectorData->getElements()[i];

                if (proxy.testElement(element->getGeometry(), nullptr, Variant(element->getMetaData()))) {
                    elements.push_back(element);
                }
            }
        }
        return elements;
    }

}

#endif
