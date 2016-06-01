#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "PackageInfo.h"
#include "PackageMetaInfo.h"

namespace carto {

    std::string PackageInfo::getName() const {
        Variant nameMetaInfo;
        if (_metaInfo) {
            nameMetaInfo = _metaInfo->getVariant().getObjectElement("name");
            if (nameMetaInfo.getType() == VariantType::VARIANT_TYPE_NULL) {
                nameMetaInfo = _metaInfo->getVariant().getObjectElement("name_en");
            }
        }

        if (nameMetaInfo.getType() == VariantType::VARIANT_TYPE_ARRAY) {
            for (int i = 0; i < nameMetaInfo.getArraySize(); i++) {
                return nameMetaInfo.getArrayElement(i).getString();
            }
        }
        else if (nameMetaInfo.getType() == VariantType::VARIANT_TYPE_STRING) {
            return nameMetaInfo.getString();
        }
        return _packageId;
    }

    std::vector<std::string> PackageInfo::getNames(const std::string& lang) const {
        Variant nameMetaInfo;
        if (_metaInfo) {
            nameMetaInfo = _metaInfo->getVariant().getObjectElement("name_" + lang);
            if (nameMetaInfo.getType() == VariantType::VARIANT_TYPE_NULL) {
                nameMetaInfo = _metaInfo->getVariant().getObjectElement("name");
            }
            if (nameMetaInfo.getType() == VariantType::VARIANT_TYPE_NULL) {
                nameMetaInfo = _metaInfo->getVariant().getObjectElement("name_en");
            }
        }

        std::vector<std::string> names;
        if (nameMetaInfo.getType() == VariantType::VARIANT_TYPE_ARRAY) {
            for (int i = 0; i < nameMetaInfo.getArraySize(); i++) {
                names.push_back(nameMetaInfo.getArrayElement(i).getString());
            }
        }
        else if (nameMetaInfo.getType() == VariantType::VARIANT_TYPE_STRING) {
            names.push_back(nameMetaInfo.getString());
        }
        else {
            names.push_back(_packageId);
        }
        return names;
    }

}

#endif
