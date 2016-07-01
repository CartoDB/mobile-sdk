#include "VectorData.h"
#include "vectorelements/VectorElement.h"

namespace carto {
    
    VectorData::VectorData(const std::vector<std::shared_ptr<VectorElement> >& elements) :
        _elements(elements), _mutex()
    {
    }

    VectorData::~VectorData() {
    }

    const std::vector<std::shared_ptr<VectorElement> >& VectorData::getElements() const {
        return _elements;
    }

}
