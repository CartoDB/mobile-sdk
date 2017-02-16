/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VARIANT_H_
#define _CARTO_VARIANT_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include <picojson/picojson.h>

namespace carto {

    namespace VariantType {
        /**
         * Variant type.
         */
        enum VariantType {
            /**
             * Null element.
             */
            VARIANT_TYPE_NULL,
            /**
             * String element.
             */
            VARIANT_TYPE_STRING,
            /**
             * Boolean element.
             */
            VARIANT_TYPE_BOOL,
            /**
             * Integer element.
             */
            VARIANT_TYPE_INTEGER,
            /**
             * Double-precision floating point element.
             */
            VARIANT_TYPE_DOUBLE,
            /**
             * Array element.
             */
            VARIANT_TYPE_ARRAY,
            /**
             * Object (dictionary) element.
             */
            VARIANT_TYPE_OBJECT
        };
    }
    
    /**
     * JSON value. Can contain JSON-style structured data, including objects and arrays.
     */
    class Variant {
    public:
        /**
         * Constructs a null Variant object.
         */
        Variant();
        /**
         * Constructs Variant object from a boolean.
         * @param boolVal The boolean value.
         */
        explicit Variant(bool boolVal);
        /**
         * Constructs Variant object from an integer.
         * @param longVal The integer value.
         */
        explicit Variant(long long longVal);
        /**
         * Constructs Variant object from a double.
         * @param doubleVal The double value.
         */
        explicit Variant(double doubleVal);
        /**
         * Constructs Variant object from a string.
         * @param str The string value.
         */
        explicit Variant(const char* str);
        /**
         * Constructs Variant object from a string.
         * @param string The string value.
         */
        explicit Variant(const std::string& string);
        /**
         * Constructs Variant object from a list of values.
         * @param array The array of JSON values.
         */
        explicit Variant(const std::vector<Variant>& array);
        /**
         * Constructs Variant object from a map of values.
         * @param object The map of JSON values.
         */
        explicit Variant(const std::map<std::string, Variant>& object);

        /**
         * Checks for equality between this and another value object.
         * @param var The other variant object.
         * @return True if equal.
         */
        bool operator == (const Variant& var) const;
        /**
         * Checks for inequality between this and another value object.
         * @param value The other variant object.
         * @return True if not equal.
         */
        bool operator != (const Variant& var) const;
        
        /**
         * Returns the type of this variant.
         * @return The type of this variant.
         */
        VariantType::VariantType getType() const;

        /**
         * Returns the string value of this variant.
         * @return The string value of the variant. If the variant is not string, empty string will be returned.
         */
        std::string getString() const;
        /**
         * Returns the boolean value of this variant.
         * @return The boolean value of the variant. If the variant is not boolean, false will be returned.
         */
        bool getBool() const;
        /**
         * Returns the integer value of this variant.
         * @return The integer value of the variant. If the variant is not integer, 0 will be returned.
         */
        long long getLong() const;
        /**
         * Returns the floating point value of this variant.
         * @return The floating point value of the variant. If the variant is integer, it will be converted to floating point. Otherwise 0.0 is be returned.
         */
        double getDouble() const;

        /**
         * Returns the number of elements in the array.
         * @return The number of elements in the array if the variant is of array type. Otherwise 0 is returned.
         */
        int getArraySize() const;
        /**
         * Returns the element of array at specified position.
         * @param idx The index of the array element to return (starting from 0).
         * @return The array element at specified position or null type if the element does not exist or the variant is not an array.
         */
        Variant getArrayElement(int idx) const;

        /**
         * Returns all the keys in the object.
         * @return The list containing all the keys of the object if the variant is of object type. Otherwise empty list is returned.
         */
        std::vector<std::string> getObjectKeys() const;
        /**
         * Returns the element of object with the specified key.
         * @param key The key of the object element to return.
         * @return The object element with the specified key or null type if the element does not exist or the variant is not an object.
         */
        Variant getObjectElement(const std::string& key) const;

        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;
        
        /**
         * Converts the variant to JSON string.
         * @return The JSON string corresponding to the variant.
         */
        std::string toString() const;

        /**
         * Converts the variant to PicoJSON value.
         * @return The PicoJSON object corresponding to the variant.
         */
        const picojson::value& toPicoJSON() const;

        /**
         * Creates a Variant object from a JSON string representation.
         * @param str The JSON string to use for the variant.
         * @return The corresponding Variant object.
         * @throws std::runtime_exception If the string parsing fails.
         */
        static Variant FromString(const std::string& str);

        /**
         * Creates Variant object from a PicoJSON value.
         * @param val The PicoJSON value to use.
         * @return The corresponding Variant object.
         */
        static Variant FromPicoJSON(const picojson::value& val);

    private:
        picojson::value _value;
    };

}

#endif
