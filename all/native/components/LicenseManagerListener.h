/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LICENSEMANAGERLISTENER_H_
#define _CARTO_LICENSEMANAGERLISTENER_H_

namespace carto {

    /**
     * An internal listener class for notifying about license update events.
     */
    class LicenseManagerListener {
    public:
        virtual ~LicenseManagerListener() { }
    
        /**
         * Called when the new license has been retrieved.
         * @param licenseKey The new license key.
         */
        virtual void onLicenseUpdated(const std::string& licenseKey) = 0;
    };
    
}

#endif
