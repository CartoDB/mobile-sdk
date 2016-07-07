#ifdef _CARTO_WKBT_SUPPORT

#include "WKBGeometryWriter.h"
#include "WKBGeometryEnums.h"
#include "Geometry.h"
#include "PointGeometry.h"
#include "LineGeometry.h"
#include "PolygonGeometry.h"
#include "MultiGeometry.h"
#include "MultiPointGeometry.h"
#include "MultiLineGeometry.h"
#include "MultiPolygonGeometry.h"
#include "core/BinaryData.h"
#include "utils/Log.h"

#include <stdexcept>

namespace carto {

    WKBGeometryWriter::Stream::Stream() :
        _data(),
        _bigEndian()
    {
    }

    const std::vector<unsigned char>& WKBGeometryWriter::Stream::data() const {
        return _data;
    }

    void WKBGeometryWriter::Stream::reserve(std::size_t count) {
        _data.reserve(_data.size() + count);
    }

    void WKBGeometryWriter::Stream::pushBigEndian(bool little) {
        _bigEndian.push(little);
    }
    
    void WKBGeometryWriter::Stream::popBigEndian() {
        _bigEndian.pop();
    }

    void WKBGeometryWriter::Stream::writeByte(unsigned char val) {
        _data.push_back(val);
    }

    void WKBGeometryWriter::Stream::writeUInt32(std::uint32_t val) {
        if (_bigEndian.top()) {
            _data.push_back(static_cast<unsigned char>((val >> 24) & 255));
            _data.push_back(static_cast<unsigned char>((val >> 16) & 255));
            _data.push_back(static_cast<unsigned char>((val >> 8) & 255));
            _data.push_back(static_cast<unsigned char>(val & 255));
        } else {
            _data.push_back(static_cast<unsigned char>(val & 255));
            _data.push_back(static_cast<unsigned char>((val >> 8) & 255));
            _data.push_back(static_cast<unsigned char>((val >> 16) & 255));
            _data.push_back(static_cast<unsigned char>((val >> 24) & 255));
        }
    }

    void WKBGeometryWriter::Stream::writeDouble(double val) {
        std::uint64_t valInt = *reinterpret_cast<std::uint64_t*>(&val);
        if (_bigEndian.top()) {
            for (int i = 7; i >= 0; i--) {
                _data.push_back(static_cast<unsigned char>((valInt >> (i * 8)) & 255));
            }
        } else {
            for (int i = 0; i < 8; i++) {
                _data.push_back(static_cast<unsigned char>((valInt >> (i * 8)) & 255));
            }
        }
    }

    WKBGeometryWriter::WKBGeometryWriter() :
        _bigEndian(false),
        _maskZM(0),
        _mutex()
    {
    }

    bool WKBGeometryWriter::getBigEndian() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _bigEndian;
    }

    void WKBGeometryWriter::setBigEndian(bool bigEndian) {
        std::lock_guard<std::mutex> lock(_mutex);
        _bigEndian = bigEndian;
    }

    bool WKBGeometryWriter::getZ() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return (_maskZM & WKB_ZMASK) != 0;
    }
    
    void WKBGeometryWriter::setZ(bool z) {
        std::lock_guard<std::mutex> lock(_mutex);
        _maskZM = (z ? WKB_ZMASK : 0);
    }

    std::shared_ptr<BinaryData> WKBGeometryWriter::writeGeometry(const std::shared_ptr<Geometry>& geometry) const {
        if (!geometry) {
            throw std::invalid_argument("Null geometry");
        }

        std::lock_guard<std::mutex> lock(_mutex);

        Stream stream;
        try {
            writeGeometry(geometry, stream);
            return std::make_shared<BinaryData>(stream.data());
        } catch (const std::exception& ex) {
            Log::Errorf("WKBGeometryWriter::writeGeometry: Exception while writing geometry: %s", ex.what());
        }
        return std::shared_ptr<BinaryData>();
    }

    void WKBGeometryWriter::writeGeometry(const std::shared_ptr<Geometry>& geometry, Stream& stream) const {
        stream.writeByte(_bigEndian ? WKB_XDR : WKB_NDR);
        stream.pushBigEndian(_bigEndian);

        if (auto point = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            std::uint32_t type = WKB_POINT | _maskZM;
            stream.writeUInt32(type);
            writePoint(point->getPos(), type, stream);
        } else if (auto line = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            std::uint32_t type = WKB_LINESTRING | _maskZM;
            stream.writeUInt32(type);
            writeRing(line->getPoses(), type, stream);
        } else if (auto polygon = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            std::uint32_t type = WKB_POLYGON | _maskZM;
            stream.writeUInt32(type);
            writeRings(polygon->getRings(), type, stream);
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            if (std::dynamic_pointer_cast<MultiPointGeometry>(geometry)) {
                stream.writeUInt32(WKB_MULTIPOINT);
            } if (std::dynamic_pointer_cast<MultiLineGeometry>(geometry)) {
                stream.writeUInt32(WKB_MULTILINESTRING);
            } if (std::dynamic_pointer_cast<MultiPolygonGeometry>(geometry)) {
                stream.writeUInt32(WKB_MULTIPOLYGON);
            } else {
                stream.writeUInt32(WKB_GEOMETRYCOLLECTION);
            }
            std::uint32_t geometryCount = multiGeometry->getGeometryCount();
            stream.writeUInt32(geometryCount);
            for (std::uint32_t i = 0; i < geometryCount; i++) {
                writeGeometry(multiGeometry->getGeometry(i), stream);
            }
        } else {
            throw std::runtime_error("Unsupported geometry type");
        }

        stream.popBigEndian();
    }

    void WKBGeometryWriter::writePoint(const MapPos& pos, std::uint32_t type, Stream& stream) const {
        stream.writeDouble(pos.getX());
        stream.writeDouble(pos.getY());
        if (type & WKB_ZMASK) {
            stream.writeDouble(pos.getZ());
        }
        if (type & WKB_MMASK) {
            stream.writeDouble(0);
        }
    }

    void WKBGeometryWriter::writeRing(const std::vector<MapPos>& ring, std::uint32_t type, Stream& stream) const {
        std::uint32_t count = static_cast<std::uint32_t>(ring.size());
        stream.reserve(4 + 16 * count);
        stream.writeUInt32(count);
        for (std::uint32_t i = 0; i < count; i++) {
            writePoint(ring[i], type, stream);
        }
    }

    void WKBGeometryWriter::writeRings(const std::vector<std::vector<MapPos> >& rings, std::uint32_t type, Stream& stream) const {
        std::uint32_t ringCount = static_cast<std::uint32_t>(rings.size());
        stream.writeUInt32(ringCount);
        for (std::uint32_t i = 0; i < ringCount; i++) {
            writeRing(rings[i], type, stream);
        }
    }

}

#endif
