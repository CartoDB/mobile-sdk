/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BINARYDATA_H_
#define _CARTO_BINARYDATA_H_

#include <memory>
#include <vector>
#include <string>
#include <mutex>

namespace carto {
    
    /**
     * A wrapper class for binary data (Blob).
     */
    class BinaryData {
    public:
        /**
         * Constructs an empty BinaryData object.
         */
        BinaryData();
        /**
         * Constructs a BinaryData object from a byte vector.
         * @param data The byte vector.
         */
        explicit BinaryData(std::vector<unsigned char> data);
        /**
         * Constructs a BinaryData object from a raw byte array.
         * @param dataPtr The raw pointer to the data.
         * @param size The size of the data in bytes.
         */
        BinaryData(const unsigned char* dataPtr, std::size_t size);

        /**
         * Check if the data is empty (size is 0).
         * @return True if the size of the data is 0. False otherwise.
         */
        bool empty() const;
        /**
         * Returns the size of the data
         * @return The size of the data.
         */
        std::size_t size() const;
        /**
         * Returns the data array.
         * @return The data array.
         */
        const unsigned char* data() const;
        /**
         * Returns the pointer to data byte vector.
         * @return The pointer to data byte vector.
         */
        std::shared_ptr<std::vector<unsigned char> > getDataPtr() const;
        
        /**
         * Checks for equality between this and another blob.
         * @param data The other data object.
         * @return True if equal.
         */
        bool operator ==(const BinaryData& data) const;
        /**
         * Checks for inequality between this and another blob.
         * @param data The other data object.
         * @return True if not equal.
         */
        bool operator !=(const BinaryData& data) const;

        /**
         * Returns the hash value of this object.
         * @return The hash value of this object.
         */
        int hash() const;

        /**
         * Creates a string representation of this map bounds object, useful for logging.
         * @return The string representation of this map bounds object.
         */
        std::string toString() const;

    private:
        std::shared_ptr<std::vector<unsigned char> > _dataPtr;
    };

}

#endif
