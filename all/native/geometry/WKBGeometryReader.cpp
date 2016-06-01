#ifdef _CARTO_WKBT_SUPPORT

#include "WKBGeometryReader.h"
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
            throw std::runtime_error("Stream array too short, can not read byte");
        }
        return _data[_offset++];
    }

    uint32_t WKBGeometryReader::Stream::readUInt32() {
        if (_offset + 4 > _data.size()) {
            throw std::runtime_error("Stream array too short, can not read 32-bit word");
        }
        uint32_t val = 0;
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
            throw std::runtime_error("Stream array too short, can not read double float");
        }
        uint64_t val = 0;
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
            return std::shared_ptr<Geometry>();
        }

        Stream stream(*wkbData->getDataPtr());
        try {
            return readGeometry(stream);
        } catch (const std::exception& ex) {
            Log::Errorf("WKBGeometryReader::readGeometry: Exception while reading geometry: %s", ex.what());
        }
        return std::shared_ptr<Geometry>();
    }

    std::shared_ptr<Geometry> WKBGeometryReader::readGeometry(Stream& stream) const {
        unsigned char bigEndian = stream.readByte();
        stream.pushBigEndian(bigEndian == wkbXDR);

        uint32_t type = stream.readUInt32();
        std::shared_ptr<Geometry> geometry;
        switch (type & ~(wkbZMask | wkbMMask)) {
            case wkbPoint: {
                geometry = std::make_shared<PointGeometry>(readPoint(stream, type));
                break;
            }
            case wkbLineString: {
                geometry = std::make_shared<LineGeometry>(readRing(stream, type));
                break;
            }
            case wkbPolygon: {
                geometry = std::make_shared<PolygonGeometry>(readRings(stream, type));
                break;
            }
            case wkbMultiPoint: {
                std::vector<std::shared_ptr<PointGeometry> > points;
                uint32_t pointCount = stream.readUInt32();
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
            case wkbMultiLineString: {
                std::vector<std::shared_ptr<LineGeometry> > lines;
                uint32_t lineCount = stream.readUInt32();
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
            case wkbMultiPolygon: {
                std::vector<std::shared_ptr<PolygonGeometry> > polygons;
                uint32_t polygonCount = stream.readUInt32();
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
            case wkbGeometryCollection: {
                std::vector<std::shared_ptr<Geometry> > geometries;
                uint32_t geometryCount = stream.readUInt32();
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
                throw std::runtime_error("Unknown geometry type"); // NOTE: not possible to continue after this
            }
        }

        stream.popBigEndian();
        return geometry;
    }

    MapPos WKBGeometryReader::readPoint(Stream& stream, uint32_t type) const {
        double x = stream.readDouble();
        double y = stream.readDouble();
        double z = 0;
        if (type & wkbZMask) {
            z = stream.readDouble();
        }
        if (type & wkbMMask) {
            stream.readDouble();
        }
        return MapPos(x, y, z);
    }

    std::vector<MapPos> WKBGeometryReader::readRing(Stream& stream, uint32_t type) const {
        uint32_t pointCount = stream.readUInt32();
        std::vector<MapPos> ring;
        ring.reserve(pointCount);
        while (pointCount-- > 0) {
            ring.push_back(readPoint(stream, type));
        }
        return ring;
    }

    std::vector<std::vector<MapPos> > WKBGeometryReader::readRings(Stream& stream, uint32_t type) const {
        uint32_t ringCount = stream.readUInt32();
        std::vector<std::vector<MapPos> > rings;
        rings.reserve(ringCount);
        while (ringCount-- > 0) {
            rings.push_back(readRing(stream, type));
        }
        return rings;
    }

}

#endif
