#include "NMLModelStyleBuilder.h"
#include "assets/DefaultNMLModelNML.h"
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
        std::lock_guard<std::mutex> lock(_DefaultNMLModelMutex);
        if (!_DefaultNMLModel) {
            _DefaultNMLModel = std::make_shared<BinaryData>(default_nmlmodel_nml, default_nmlmodel_nml_len);
        }
        return _DefaultNMLModel;
    }

    std::shared_ptr<BinaryData> NMLModelStyleBuilder::_DefaultNMLModel;
    std::mutex NMLModelStyleBuilder::_DefaultNMLModelMutex;
    
}
