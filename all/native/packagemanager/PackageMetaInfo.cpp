#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "PackageMetaInfo.h"

namespace carto {

    PackageMetaInfo::PackageMetaInfo(const Variant& var) :
        _variant(var)
    {
    }

    const Variant& PackageMetaInfo::getVariant() const {
        return _variant;
    }

}

#endif
