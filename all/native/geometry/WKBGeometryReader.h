/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_WKBGEOMETRYREADER_H_
#define _CARTO_WKBGEOMETRYREADER_H_

#ifdef _CARTO_WKBT_SUPPORT

#include "core/MapPos.h"

#include <memory>
#include <vector>
#include <stack>
#include <cstddef>

namespace carto {
    class BinaryData;
    class Geometry;

    /**
     * A WKB reader. Reads binary version of the Well Known Text representation of the geometry.
     */
    class WKBGeometryReader {
    public:
        /**
         * Constructs a new WKBGeometryReader object.
         */
        WKBGeometryReader();

        /**
         * Reads geometry from the specified WKB data.
         * @param wkbData The WKB data to read.
         * @return The geometry read from the data vector. Null if reading failed.
         * @throws std::runtime_error If reading fails.
         */
        std::shared_ptr<Geometry> readGeometry(const std::shared_ptr<BinaryData>& wkbData) const;

    private:
        struct Stream {
            Stream(const std::vector<unsigned char>& data);

            void pushBigEndian(bool little);
            void popBigEndian();

            unsigned char readByte();
            std::uint32_t readUInt32();
            double readDouble();
        
        private:
            const std::vector<unsigned char>& _data;
            std::size_t _offset;
            std::stack<bool> _bigEndian;
        };

        std::shared_ptr<Geometry> readGeometry(Stream& stream) const;
        MapPos readPoint(Stream& stream, std::uint32_t type) const;
        std::vector<MapPos> readRing(Stream& stream, std::uint32_t type) const;
        std::vector<std::vector<MapPos> > readRings(Stream& stream, std::uint32_t type) const;
    };

}

#endif

#endif
