#include "Geometry.h"

namespace carto { namespace mvt {
    LineGeometry::Vertices LineGeometry::getMidPoints() const {
        Vertices midPoints;
        midPoints.reserve(getVerticesList().size());
        for (const Vertices& vertices : getVerticesList()) {
            if (vertices.empty()) {
                continue;
            }
            if (vertices.size() == 1) {
                midPoints.push_back(vertices.front());
                continue;
            }
            float pos = 0;
            for (std::size_t i = 1; i < vertices.size(); i++) {
                float len = cglib::length(vertices[i] - vertices[i - 1]);
                pos += len * 0.5f;
            }
            for (std::size_t i = 1; i < vertices.size(); i++) {
                float len = cglib::length(vertices[i] - vertices[i - 1]);
                if (pos <= len) {
                    float t = pos / len;
                    midPoints.push_back(vertices[i] * t + vertices[i - 1] * (1 - t));
                    break;
                }
                pos -= len;
            }
        }
        return midPoints;
    }

    PolygonGeometry::VerticesList PolygonGeometry::getClosedOuterRings(bool clip) const {
        auto pointClass = [](const cglib::vec2<float>& v) -> bool {
            int flags = 0;
            if (v(0) < 0) flags |= 1;
            if (v(0) > 1) flags |= 2;
            if (v(1) < 0) flags |= 4;
            if (v(1) > 1) flags |= 8;
            return flags;
        };

        VerticesList rings;
        rings.reserve(getPolygonList().size());
        for (const VerticesList& polygon : getPolygonList()) {
            Vertices ring;
            if (!polygon.empty()) {
                ring = polygon.front();
                if (!ring.empty()) {
                    ring.push_back(ring.front());
                }

                if (clip) {
                    for (std::size_t i = 0; i < ring.size(); i++) {
                        int flags1 = pointClass(ring[i]);
                        if (!flags1) {
                            continue;
                        }

                        if (i > 0) {
                            rings.push_back(Vertices(ring.begin(), ring.begin() + i + 1));
                            ring.erase(ring.begin(), ring.begin() + i);
                        }

                        for (i = 1; i < ring.size(); i++) {
                            int flags2 = pointClass(ring[i]);
                            if (!(flags1 & flags2)) {
                                break;
                            }
                            flags1 = flags2;
                        }
                        ring.erase(ring.begin(), ring.begin() + i - 1);
                        i = 0;
                    }
                }
            }
            if (ring.size() > 1) {
                rings.push_back(std::move(ring));
            }
        }
        return rings;
    }

    PolygonGeometry::Vertices PolygonGeometry::getCenterPoints() const {
        Vertices centerPoints;
        centerPoints.reserve(getPolygonList().size());
        for (const VerticesList& verticesList : getPolygonList()) {
            if (!verticesList.empty()) {
                const Vertices& vertices = verticesList.front();
                cglib::vec2<float> center(0, 0);
                for (const cglib::vec2<float>& vertex : vertices) {
                    center += vertex * (1.0f / vertices.size());
                }
                centerPoints.push_back(center);
            }
        }
        return centerPoints;
    }

    PolygonGeometry::Vertices PolygonGeometry::getSurfacePoints() const {
        Vertices surfacePoints;
        surfacePoints.reserve(getPolygonList().size());
        for (const VerticesList& verticesList : getPolygonList()) {
            if (!verticesList.empty()) {
                const Vertices& vertices = verticesList.front();

                float midY = 0;
                if (!vertices.empty()) {
                    auto minmaxIt = std::minmax_element(vertices.begin(), vertices.end(), [](const cglib::vec2<float>& p1, const cglib::vec2<float>& p2) {
                        return p1(1) < p2(1);
                    });
                    midY = ((*minmaxIt.first)(1) + (*minmaxIt.second)(1)) * 0.5f;
                }

                std::vector<cglib::vec2<float>> points;
                for (std::size_t i = 0; i < vertices.size(); i++) {
                    cglib::vec2<float> p0 = vertices[i];
                    cglib::vec2<float> p1 = vertices[(i + 1) % vertices.size()];
                    if (p0(1) > p1(1)) {
                        std::swap(p0, p1);
                    }
                    if (p0(1) <= midY && p1(1) >= midY) {
                        float t = (p1(1) > p0(1) ? (midY - p0(1)) / (p1(1) - p0(1)) : 0.5f);
                        cglib::vec2<float> pt = p0 + (p1 - p0) * t;
                        auto it = std::upper_bound(points.begin(), points.end(), pt, [](const cglib::vec2<float>& p1, const cglib::vec2<float>& p2) {
                            return p1(0) < p2(0);
                        });
                        points.insert(it, pt);
                    }
                }

                cglib::vec2<float> bestPoint(0, 0);
                float bestSegmentLength = -std::numeric_limits<float>::infinity();
                for (std::size_t i = 0; i + 1 < points.size(); i += 2) {
                    float segmentLength = cglib::length(points[i + 1] - points[i]);
                    if (segmentLength > bestSegmentLength) {
                        bestSegmentLength = segmentLength;
                        bestPoint = (points[i] + points[i + 1]) * 0.5f;
                    }
                }

                surfacePoints.push_back(bestPoint);
            }
        }
        return surfacePoints;
    }

} }
