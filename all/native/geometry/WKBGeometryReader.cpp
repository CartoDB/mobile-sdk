#ifdef _CARTO_WKBT_SUPPORT

#include "WKBGeometryReader.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "geometry/WKBGeometryEnums.h"
#include "utils/Log.h"

#include <stdexcept>

namespace carto {

    WKBGeometryReader::Stream::Stream(const std::vector<unsigned char>& data) :
        _data(data),
        _offset(0),
        _bigEndian()
    {
    }

    void WKBGeometryReader::Stream::pushBigEndian(bool little) {
        _bigEndian.push(little);
    }

    void WKBGeometryReader::Stream::popBigEndian() {
        _bigEndian.pop();
    }

    unsigned char WKBGeometryReader::Stream::readByte() {
        if (_offset + 1 > _data.size()) {
            throw ParseException("Stream array too short, can not read byte");
        }
        return _data[_offset++];
    }

    std::uint32_t WKBGeometryReader::Stream::readUInt32() {
        if (_offset + 4 > _data.size()) {
            throw ParseException("Stream array too short, can not read 32-bit word");
        }
        std::uint32_t val = 0;
        if (_bigEndian.top()) {
            val = _data[_offset + 0];
            val = (val << 8) | _data[_offset + 1];
            val = (val << 8) | _data[_offset + 2];
            val = (val << 8) | _data[_offset + 3];
        } else {
            val = _data[_offset + 3];
            val = (val << 8) | _data[_offset + 2];
            val = (val << 8) | _data[_offset + 1];
            val = (val << 8) | _data[_offset + 0];
        }
        _offset += 4;
        return val;
    }

    double WKBGeometryReader::Stream::readDouble() {
        if (_offset + 8 > _data.size()) {
            throw ParseException("Stream array too short, can not read double float");
        }
        std::uint64_t val = 0;
        if (_bigEndian.top()) {
            for (int i = 0; i < 8; i++) {
                val = (val << 8) | _data[_offset + i];
            }
        } else {
            for (int i = 7; i >= 0; i--) {
                val = (val << 8) | _data[_offset + i];
            }
        }
        _offset += 8;
        return *reinterpret_cast<double*>(&val);
    }

    WKBGeometryReader::WKBGeometryReader() {
    }

    std::shared_ptr<Geometry> WKBGeometryReader::readGeometry(const std::shared_ptr<BinaryData>& wkbData) const {
        if (!wkbData) {
            throw NullArgumentException("Null wkbData");
        }

        Stream stream(*wkbData->getDataPtr());
        return readGeometry(stream);
    }

    std::shared_ptr<Geometry> WKBGeometryReader::readGeometry(Stream& stream) const {
        unsigned char bigEndian = stream.readByte();
        stream.pushBigEndian(bigEndian == WKB_XDR);

        std::uint32_t type = stream.readUInt32();
        std::shared_ptr<Geometry> geometry;
        switch (type & ~(WKB_ZMASK | WKB_MMASK)) {
            case WKB_POINT: {
                geometry = std::make_shared<PointGeometry>(readPoint(stream, type));
                break;
            }
            case WKB_LINESTRING: {
                geometry = std::make_shared<LineGeometry>(readRing(stream, type));
                break;
            }
            case WKB_POLYGON: {
                geometry = std::make_shared<PolygonGeometry>(readRings(stream, type));
                break;
            }
            case WKB_MULTIPOINT: {
                std::vector<std::shared_ptr<PointGeometry> > points;
                std::uint32_t pointCount = stream.readUInt32();
                points.reserve(pointCount);
                while (pointCount-- > 0) {
                    if (auto point = std::dynamic_pointer_cast<PointGeometry>(readGeometry(stream))) {
                        points.push_back(point);
                    } else {
                        Log::Error("WKBGeometryReader::readGeometry: Illegal geometry type when reading multipoint");
                    }
                }
                geometry = std::make_shared<MultiPointGeometry>(points);
                break;
            }
            case WKB_MULTILINESTRING: {
                std::vector<std::shared_ptr<LineGeometry> > lines;
                std::uint32_t lineCount = stream.readUInt32();
                lines.reserve(lineCount);
                while (lineCount-- > 0) {
                    if (auto line = std::dynamic_pointer_cast<LineGeometry>(readGeometry(stream))) {
                        lines.push_back(line);
                    } else {
                        Log::Error("WKBGeometryReader::readGeometry: Illegal geometry type when reading multilinestring");
                    }
                }
                geometry = std::make_shared<MultiLineGeometry>(lines);
                break;
            }
            case WKB_MULTIPOLYGON: {
                std::vector<std::shared_ptr<PolygonGeometry> > polygons;
                std::uint32_t polygonCount = stream.readUInt32();
                polygons.reserve(polygonCount);
                while (polygonCount-- > 0) {
                    if (auto polygon = std::dynamic_pointer_cast<PolygonGeometry>(readGeometry(stream))) {
                        polygons.push_back(polygon);
                    } else {
                        Log::Error("WKBGeometryReader::readGeometry: Illegal geometry type when reading multipolygon");
                    }
                }
                geometry = std::make_shared<MultiPolygonGeometry>(polygons);
                break;
            }
            case WKB_GEOMETRYCOLLECTION: {
                std::vector<std::shared_ptr<Geometry> > geometries;
                std::uint32_t geometryCount = stream.readUInt32();
                geometries.reserve(geometryCount);
                while (geometryCount-- > 0) {
                    if (auto geometry = readGeometry(stream)) {
                        geometries.push_back(geometry);
                    }
                }
                geometry = std::make_shared<MultiGeometry>(geometries);
                break;
            }
            default: {
                throw ParseException("Unknown geometry type"); // NOTE: not possible to continue after this
            }
        }

        stream.popBigEndian();
        return geometry;
    }

    MapPos WKBGeometryReader::readPoint(Stream& stream, std::uint32_t type) const {
        double x = stream.readDouble();
        double y = stream.readDouble();
        double z = 0;
        if (type & WKB_ZMASK) {
            z = stream.readDouble();
        }
        if (type & WKB_MMASK) {
            stream.readDouble();
        }
        return MapPos(x, y, z);
    }

    std::vector<MapPos> WKBGeometryReader::readRing(Stream& stream, std::uint32_t type) const {
        std::uint32_t pointCount = stream.readUInt32();
        std::vector<MapPos> ring;
        ring.reserve(pointCount);
        while (pointCount-- > 0) {
            ring.push_back(readPoint(stream, type));
        }
        return ring;
    }

    std::vector<std::vector<MapPos> > WKBGeometryReader::readRings(Stream& stream, std::uint32_t type) const {
        std::uint32_t ringCount = stream.readUInt32();
        std::vector<std::vector<MapPos> > rings;
        rings.reserve(ringCount);
        while (ringCount-- > 0) {
            rings.push_back(readRing(stream, type));
        }
        return rings;
    }

}

#endif
