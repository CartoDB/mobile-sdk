/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PACKAGEMETAINFO_H_
#define _CARTO_PACKAGEMETAINFO_H_

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "core/Variant.h"

namespace carto {

    /**
     * Package meta info element. Can contain JSON-style structured data, including objects and arrays.
     */
    class PackageMetaInfo {
    public:
        /**
         * Constructs a new PackageMetaInfo object from existing JSON value.
         * @param var The variant to use.
         */
        explicit PackageMetaInfo(const Variant& var);

        /**
         * Returns the underlying variant.
         * @return The value of the metainfo.
         */
        const Variant& getVariant() const;

    private:
        Variant _variant;
    };

}

#endif

#endif
