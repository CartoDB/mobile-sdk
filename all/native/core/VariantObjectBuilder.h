/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VARIANTOBJECTBUILDER_H_
#define _CARTO_VARIANTOBJECTBUILDER_H_

#include "core/Variant.h"

#include <mutex>
#include <map>

namespace carto {

    /**
     * A helper class for building variant objects.
     */
    class VariantObjectBuilder {
    public:
        /**
         * A default constructor.
         */
        VariantObjectBuilder();
        
        /**
         * Sets a specified string value.
         * @param key The key to set.
         * @param str The string value.
         */
        void setString(const std::string& key, const std::string& str);
        /**
         * Sets a specified boolean value.
         * @param key The key to set.
         * @param val The boolean value.
         */
        void setBool(const std::string& key, bool val);
        /**
         * Sets a specified long integer value.
         * @param key The key to set.
         * @param val The integer value.
         */
        void setLong(const std::string& key, long long val);
        /**
         * Sets a specified double precision floating point value.
         * @param key The key to set.
         * @param val The floating point value.
         */
        void setDouble(const std::string& key, double val);
        /**
         * Sets a specified variant value.
         * @param key The key to set.
         * @param var The variant to add.
         */
        void setVariant(const std::string& key, const Variant& var);

        /**
         * Builds a new object variant from the added elements.
         * @return The corresponding variant object.
         */
        Variant buildVariant() const;

    private:
        std::map<std::string, Variant> _elementMap;
        mutable std::mutex _mutex;
    };

}

#endif
