#include "NMLModelStyleBuilder.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "styles/NMLModelStyle.h"

namespace carto {

    NMLModelStyleBuilder::NMLModelStyleBuilder() :
        StyleBuilder(),
        _modelAsset(GetDefaultModelAsset())
    {
    }
    
    NMLModelStyleBuilder::~NMLModelStyleBuilder() {
    }
        
    std::shared_ptr<BinaryData> NMLModelStyleBuilder::getModelAsset() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _modelAsset;
    }
    
    void NMLModelStyleBuilder::setModelAsset(const std::shared_ptr<BinaryData>& modelAsset) {
        if (!modelAsset) {
            throw NullArgumentException("Null modelAsset");
        }

        std::lock_guard<std::mutex> lock(_mutex);
        _modelAsset = modelAsset;
    }
        
    std::shared_ptr<NMLModelStyle> NMLModelStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::make_shared<NMLModelStyle>(_color, _modelAsset);
    }
    
    std::shared_ptr<BinaryData> NMLModelStyleBuilder::GetDefaultModelAsset() {
        return std::shared_ptr<BinaryData>();
    }
    
}
