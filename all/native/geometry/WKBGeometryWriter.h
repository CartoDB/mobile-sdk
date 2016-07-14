/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_WKBGEOMETRYWRITER_H_
#define _CARTO_WKBGEOMETRYWRITER_H_

#ifdef _CARTO_WKBT_SUPPORT

#include "core/MapPos.h"

#include <memory>
#include <mutex>
#include <cstddef>
#include <vector>
#include <stack>

namespace carto {
    class BinaryData;
    class Geometry;

    /**
     * A WKB writer. Generates binary version of the Well Known Text (WKT) representation of the geometry.
     * Supports both 2D and 3D coordinate serialization and big- and little-endian representations.
     */
    class WKBGeometryWriter {
    public:
        /**
         * Constructs a new WKBGeometryWriter objects with default settings.
         */
        WKBGeometryWriter();

        /**
         * Returns the endianness of output format.
         * @return True when big-endian representation is used, false if little-endian. The default is little-endian.
         */
        bool getBigEndian() const;
        /**
         * Sets the endianness of output format.
         * @param bigEndian True if big-endian representation should be used, false if little-endian.
         */
        void setBigEndian(bool bigEndian);
        
        /**
         * Returns the state of Z coordinate serialization.
         * @return True if Z coordinate is stored in WKB output, false otherwise. The default is false.
         */
        bool getZ() const;
        /**
         * Sets the state of Z coordinate serialization.
         * @param z True when Z coordinate should be serialized, false otherwise.
         */
        void setZ(bool z);

        /**
         * Creates a WKB data corresponding to the specified geometry.
         * @param geometry The geometry to write.
         * @return The corresponding WKB data.
         * @throws std::runtime_error If writing fails.
         */
        std::shared_ptr<BinaryData> writeGeometry(const std::shared_ptr<Geometry>& geometry) const;

    private:
        struct Stream {
            Stream();

            const std::vector<unsigned char>& data() const;
            void reserve(std::size_t count);

            void pushBigEndian(bool little);
            void popBigEndian();

            void writeByte(unsigned char val);
            void writeUInt32(uint32_t val);
            void writeDouble(double val);

        private:
            std::vector<unsigned char> _data;
            std::stack<bool> _bigEndian;
        };

        void writeGeometry(const std::shared_ptr<Geometry>& geometry, Stream& stream) const;
        void writePoint(const MapPos& pos, std::uint32_t type, Stream& stream) const;
        void writeRing(const std::vector<MapPos>& ring, std::uint32_t type, Stream& stream) const;
        void writeRings(const std::vector<std::vector<MapPos> >& rings, std::uint32_t type, Stream& stream) const;

        bool _bigEndian;
        std::uint32_t _maskZM;

        mutable std::mutex _mutex;
    };

}

#endif

#endif
