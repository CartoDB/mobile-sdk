#include "NMLModelStyle.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"

#include <nml/Package.h>

namespace carto {

    NMLModelStyle::NMLModelStyle(const Color& color, const std::shared_ptr<BinaryData>& modelAsset) :
        Style(color),
        _sourceModel()
    {
        if (!modelAsset) {
            throw NullArgumentException("Null modelAsset");
        }

        std::shared_ptr<std::vector<unsigned char> > data = modelAsset->getDataPtr();
        protobuf::message modelMsg(data->data(), data->size());
        _sourceModel = std::make_shared<nml::Model>(modelMsg);
    }
    
    NMLModelStyle::~NMLModelStyle() {
    }
    
    const std::shared_ptr<nml::Model>& NMLModelStyle::getSourceModel() const {
        return _sourceModel;
    }
    
}
