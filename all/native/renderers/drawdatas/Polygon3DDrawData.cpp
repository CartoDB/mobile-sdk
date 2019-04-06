#include "Polygon3DDrawData.h"
#include "core/MapPos.h"
#include "geometry/PolygonGeometry.h"
#include "graphics/utils/GLContext.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "styles/Polygon3DStyle.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/Log.h"
#include "vectorelements/Polygon3D.h"

#include <cmath>
#include <cstdlib>
#include <numeric>

#include <tesselator.h>

namespace carto {

    Polygon3DDrawData::Polygon3DDrawData(const Polygon3D& polygon3D, const Polygon3DStyle& style, const Projection& projection, const ProjectionSurface& projectionSurface) :
        VectorElementDrawData(style.getColor()),
        _sideColor(GetPremultipliedColor(style.getSideColor())),
        _boundingBox(cglib::bbox3<double>::smallest()),
        _coords(),
        _normals(),
        _attribs()
    {
        const std::vector<MapPos>& poses = polygon3D.getGeometry()->getPoses();
        const std::vector<std::vector<MapPos> >& holes = polygon3D.getGeometry()->getHoles();
        float height = polygon3D.getHeight();
        
        // Create tesselator
        TESSalloc ma;
        ma.memalloc = [](void* userData, unsigned int size) { return malloc(size); };
        ma.memfree = [](void* userData, void* ptr) { free(ptr); };
        ma.extraVertices = 256;
        TESStesselator* tessPtr = tessNewTess(&ma);
        if (!tessPtr) {
            Log::Error("Polygon3DDrawData::Polygon3DDrawData: Failed to created tesselator!");
            return;
        }
        std::shared_ptr<TESStesselator> tess(tessPtr, tessDeleteTess);
    
        // Prepare polygon exterior
        std::vector<std::vector<MapPos> > ringsInternalPoses(1 + holes.size());
        ringsInternalPoses[0].reserve(poses.size());
        
        std::vector<double> posesArray(poses.size() * 3);
        for (std::size_t i = 0; i < poses.size(); i++) {
            MapPos pos = poses[i];
            pos.setZ(height);
            MapPos internalPos = projection.toInternal(pos);
            posesArray[i * 3 + 0] = internalPos.getX();
            posesArray[i * 3 + 1] = internalPos.getY();
            posesArray[i * 3 + 2] = internalPos.getZ();
            ringsInternalPoses[0].push_back(internalPos);
        }
        tessAddContour(tess.get(), 3, posesArray.data(), sizeof(double) * 3, static_cast<unsigned int>(poses.size()));
    
        // Prepare polygon holes
        std::vector<std::vector<double> > holesArray(holes.size());
        for (std::size_t n = 0; n < holes.size(); n++) {
            const std::vector<MapPos>& hole = holes[n];
            std::vector<double>& holeArray = holesArray[n];
            holeArray.resize(hole.size() * 3);
            for (std::size_t i = 0; i < hole.size(); i++) {
                MapPos pos = hole[i];
                pos.setZ(height);
                MapPos internalPos = projection.toInternal(pos);
                holeArray[i * 3 + 0] = internalPos.getX();
                holeArray[i * 3 + 1] = internalPos.getY();
                holeArray[i * 3 + 2] = internalPos.getZ();
                ringsInternalPoses[n + 1].push_back(internalPos);
            }
            tessAddContour(tess.get(), 3, holeArray.data(), sizeof(double) * 3, static_cast<unsigned int>(hole.size()));
        }
    
        // Triangulate
        double normal[3] = { 0, 0, 1 };
        if (!tessTesselate(tess.get(), TESS_WINDING_ODD, TESS_POLYGONS, 3, 3, normal)) {
            Log::Error("Polygon3DDrawData::Polygon3DDrawData: Failed to triangulate 3d polygon!");
            return;
        }
        const double* roofCoords = tessGetVertices(tess.get());
        const int* roofElements = tessGetElements(tess.get());
        std::size_t roofVertexCount = tessGetVertexCount(tess.get());
        std::size_t roofElementCount = tessGetElementCount(tess.get());
    
        // Do projection-surface based tesselation
        std::vector<MapPos> roofInternalPoses;
        roofInternalPoses.reserve(roofVertexCount);
        for (std::size_t i = 0; i < roofVertexCount; i++) {
            roofInternalPoses.emplace_back(roofCoords[i * 3 + 0], roofCoords[i * 3 + 1], roofCoords[i * 3 + 2]);
        }
        std::vector<unsigned int> roofIndices;
        roofIndices.reserve(roofElementCount * 3);
        for (std::size_t i = 0; i < roofElementCount * 3; i += 3) {
            unsigned int i0 = roofElements[i + 0];
            unsigned int i1 = roofElements[i + 1];
            unsigned int i2 = roofElements[i + 2];
            if (i0 != TESS_UNDEF && i1 != TESS_UNDEF && i2 != TESS_UNDEF) {
                projectionSurface.tesselateTriangle(i0, i1, i2, roofIndices, roofInternalPoses);
            }
        }

        // Tesselate rings
        for (std::vector<MapPos>& ringInternalPoses : ringsInternalPoses) {
            std::vector<MapPos> tesselatedPoses;
            tesselatedPoses.reserve(ringInternalPoses.size());
            for (std::size_t i = 0; i < ringInternalPoses.size(); i++) {
                if (!tesselatedPoses.empty()) {
                    tesselatedPoses.pop_back();
                }
                projectionSurface.tesselateSegment(ringInternalPoses[i], ringInternalPoses[(i + 1) % ringInternalPoses.size()], tesselatedPoses);
            }
            std::swap(ringInternalPoses, tesselatedPoses);
        }
        
        // Reserve space for roof and sides
        std::size_t edgeVertexCount = std::accumulate(ringsInternalPoses.begin(), ringsInternalPoses.end(), std::size_t(0), [](std::size_t count, const std::vector<MapPos>& ringInternalPoses) { return count + ringInternalPoses.size(); });
        _coords.reserve(roofIndices.size() + edgeVertexCount * 6);
        _normals.reserve(roofIndices.size() + edgeVertexCount * 6);
        
        // Convert triangulator output to coord array
        for (std::size_t i = 0; i < roofIndices.size(); i++) {
            std::size_t index = roofIndices[i];
            _coords.push_back(projectionSurface.calculatePosition(roofInternalPoses[index]));
            _normals.push_back(cglib::vec3<float>::convert(projectionSurface.calculateNormal(roofInternalPoses[index])));
            _attribs.push_back(1);
        }
        
        // Calculate sides
        for (std::size_t n = 0; n < ringsInternalPoses.size(); n++) {
            const std::vector<MapPos>& ringInternalPoses = ringsInternalPoses[n];

            // Calculate vertex orientation of the polygon
            bool clockWise = GeomUtils::IsConcavePolygonClockwise(ringInternalPoses);
            // If calculating a whole, reverse the direction
            bool flipOrder = (n > 0 ? !clockWise : clockWise);
            
            for (std::size_t j = 1; j < ringInternalPoses.size(); j++) {
                const MapPos& internalPos0 = ringInternalPoses[(flipOrder ? j : j - 1)];
                const MapPos& internalPos1 = ringInternalPoses[(flipOrder ? j - 1 : j)];
                cglib::vec3<double> p0r = projectionSurface.calculatePosition(internalPos0);
                cglib::vec3<double> p1r = projectionSurface.calculatePosition(internalPos1);
                cglib::vec3<double> p0b = projectionSurface.calculatePosition(MapPos(internalPos0.getX(), internalPos0.getY(), 0));
                cglib::vec3<double> p1b = projectionSurface.calculatePosition(MapPos(internalPos1.getX(), internalPos1.getY(), 0));

                // Add coordinates for 2 triangles
                _coords.push_back(p0r);
                _coords.push_back(p0b);
                _coords.push_back(p1r);
                _coords.push_back(p0b);
                _coords.push_back(p1b);
                _coords.push_back(p1r);

                // Calculate side normal
                cglib::vec3<double> normal = projectionSurface.calculateNormal(internalPos0);
                cglib::vec3<double> sideVec = projectionSurface.calculateVector(internalPos0, internalPos1 - internalPos0);
                cglib::vec3<float> sideNormal = cglib::vec3<float>::convert(cglib::unit(cglib::vector_product(sideVec, normal)));

                // Add normal for each vertex
                for (int k = 0; k < 6; k++) {
                    _normals.push_back(sideNormal);
                    _attribs.push_back(0);
                }
            }
        }

        // Update bounding box based on calculated coordinates
        _boundingBox.add(_coords.begin(), _coords.end());
        
        // Do sanity check on the size
        if (_coords.size() > GLContext::MAX_VERTEXBUFFER_SIZE) {
            Log::Error("Polygon3DDrawData::Polygon3DDrawData: Maximum buffer size exceeded, 3d polygon can't be drawn");
        }
    }
    
    Polygon3DDrawData::~Polygon3DDrawData() {
    }

    const Color& Polygon3DDrawData::getSideColor() const {
        return _sideColor;
    }
    
    const cglib::bbox3<double>& Polygon3DDrawData::getBoundingBox() const {
        return _boundingBox;
    }

    const std::vector<cglib::vec3<double> >& Polygon3DDrawData::getCoords() const {
        return _coords;
    }
    
    const std::vector<cglib::vec3<float> >& Polygon3DDrawData::getNormals() const {
        return _normals;
    }
    
    const std::vector<unsigned char>& Polygon3DDrawData::getAttribs() const {
        return _attribs;
    }
    
    void Polygon3DDrawData::offsetHorizontally(double offset) {
        for (cglib::vec3<double>& pos : _coords) {
            pos(0) += offset;
        }
        setIsOffset(true);
    }
    
}
