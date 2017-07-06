#ifdef _CARTO_SEARCH_SUPPORT

#include "VectorDataSourceSearchService.h"
#include "core/MapEnvelope.h"
#include "components/Exceptions.h"
#include "datasources/VectorDataSource.h"
#include "graphics/ViewState.h"
#include "renderers/components/CullState.h"
#include "geometry/Geometry.h"
#include "search/SearchProxy.h"
#include "projections/Projection.h"
#include "vectorelements/VectorElement.h"
#include "utils/Log.h"

namespace carto {

    VectorDataSourceSearchService::VectorDataSourceSearchService(const std::shared_ptr<VectorDataSource>& dataSource) :
        _dataSource(dataSource)
    {
        if (!dataSource) {
            throw NullArgumentException("Null dataSource");
        }
    }

    VectorDataSourceSearchService::~VectorDataSourceSearchService() {
    }

    const std::shared_ptr<VectorDataSource>& VectorDataSourceSearchService::getDataSource() const {
        return _dataSource;
    }

    std::vector<std::shared_ptr<VectorElement> > VectorDataSourceSearchService::findElements(const std::shared_ptr<SearchRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        SearchProxy proxy(request, _dataSource->getDataExtent());
        MapBounds searchBounds = proxy.getSearchBounds();
        auto cullState = std::make_shared<CullState>(MapEnvelope(searchBounds), ViewState());

        std::vector<std::shared_ptr<VectorElement> > elements;
        if (std::shared_ptr<VectorData> vectorData = _dataSource->loadElements(cullState)) {
            for (std::size_t i = 0; i < vectorData->getElements().size(); i++) {
                const std::shared_ptr<VectorElement>& element = vectorData->getElements()[i];

                if (proxy.testElement(element->getGeometry(), _dataSource->getProjection(), Variant(element->getMetaData()))) {
                    elements.push_back(element);
                }
            }
        }
        return elements;
    }

}

#endif
