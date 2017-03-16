/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VARIANTARRAYBUILDER_H_
#define _CARTO_VARIANTARRAYBUILDER_H_

#include "core/Variant.h"

#include <mutex>
#include <vector>

namespace carto {

    /**
     * A helper class for building variant arrays.
     */
    class VariantArrayBuilder {
    public:
        /**
         * A default constructor.
         */
        VariantArrayBuilder();
        
        /**
         * Adds a new string to the array as the last element.
         * @param str The string to add.
         */
        void addString(const std::string& str);
        /**
         * Adds a new boolean to the array as the last element.
         * @param val The boolean to add.
         */
        void addBool(bool val);
        /**
         * Adds a new long integer to the array as the last element.
         * @param val The integer to add.
         */
        void addLong(long long val);
        /**
         * Adds a new double floating point value to the array as the last element.
         * @param val The floating point value to add.
         */
        void addDouble(double val);
        /**
         * Adds a new variant to the array as the last element.
         * @param var The variant to add.
         */
        void addVariant(const Variant& var);

        /**
         * Builds a new array variant from the added elements.
         * @return The corresponding variant object.
         */
        Variant buildVariant() const;

    private:
        std::vector<Variant> _elements;
        mutable std::mutex _mutex;
    };

}

#endif
