#ifndef _CARTO_EMSCRIPTENMAPLICENSEMANAGERLISTENER_H_
#define _CARTO_EMSCRIPTENMAPLICENSEMANAGERLISTENER_H_

#include <string>
#include "components/LicenseManagerListener.h"

namespace carto {
    class MapLicenseManagerListener : public LicenseManagerListener {
    public:
        void onLicenseUpdated(const std::string& licenseKey);
    };
}

#endif
