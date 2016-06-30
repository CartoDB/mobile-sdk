#include "Polygon3DDrawData.h"
#include "core/MapPos.h"

#include "geometry/PolygonGeometry.h"
#include "projections/Projection.h"
#include "styles/Polygon3DStyle.h"
#include "utils/Const.h"
#include "utils/GeomUtils.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"
#include "vectorelements/Polygon3D.h"

#include <cmath>
#include <cstdlib>
#include <tesselator.h>

namespace {
    
    void* polygon3DStdAlloc(void* userData, unsigned int size) {
        return malloc(size);
    }
    
    void polygon3DStdFree(void* userData, void* ptr) {
        free(ptr);
    }
    
}

namespace carto {

    Polygon3DDrawData::Polygon3DDrawData(const Polygon3D& polygon3D, const Polygon3DStyle& style, const Projection& projection) :
        VectorElementDrawData(style.getColor()),
        _sideColor(GetPremultipliedColor(style.getSideColor())),
        _boundingBox(cglib::bbox3<double>::smallest()),
        _coords(),
        _normals()
    {
        // Triangulate polygon roof
        TESSalloc ma;
        ma.memalloc = polygon3DStdAlloc;
        ma.memfree = polygon3DStdFree;
        ma.extraVertices = 256;
        TESStesselator* tess = tessNewTess(&ma);
    
        // Prepare polygon exterior, calculate bounding box
        const std::vector<MapPos>& poses = polygon3D.getGeometry()->getPoses();
        std::size_t edgeVertexCount = poses.size();
        std::vector<double> posesArray(poses.size() * 2);
        for (std::size_t i = 0; i < poses.size() * 2; i += 2) {
            std::size_t index = i / 2;
            const MapPos& internalPos = projection.toInternal(poses[index]);
            posesArray[i + 0] = internalPos.getX();
            posesArray[i + 1] = internalPos.getY();
            _boundingBox.add(cglib::vec3<double>(internalPos.getX(), internalPos.getY(), internalPos.getZ()));
        }
        tessAddContour(tess, 2, posesArray.data(), sizeof(double) * 2, static_cast<unsigned int>(poses.size()));
    
        // Prepare polygon holes
        const std::vector<std::vector<MapPos> >& holes = polygon3D.getGeometry()->getHoles();
        std::vector<std::vector<double> > holesArray(holes.size() * 2);
        for (std::size_t n = 0; n < holes.size(); n++) {
            const std::vector<MapPos>& hole = holes[n];
            edgeVertexCount += hole.size();
            std::vector<double>& holeArray = holesArray[n];
            holeArray.resize(hole.size() * 2);
            for (std::size_t i = 0; i < hole.size() * 2; i += 2) {
                std::size_t index = i / 2;
                const MapPos& internalPos = projection.toInternal(hole[index]);
                holeArray[i + 0] = internalPos.getX();
                holeArray[i + 1] = internalPos.getY();
                _boundingBox.add(cglib::vec3<double>(internalPos.getX(), internalPos.getY(), internalPos.getZ()));
            }
            tessAddContour(tess, 2, holeArray.data(), sizeof(double) * 2, static_cast<unsigned int>(hole.size()));
        }
    
        // Triangulate
        double normal[3] = { 0, 0, 1 };
        if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, MAX_INDICES_PER_ELEMENT, 2, normal)) {
            Log::Error("Polygon3DDrawData::Polygon3DDrawData: Failed to triangulate 3d polygon!");
            tessDeleteTess(tess);
            return;
        }
    
        const double* roofCoords = tessGetVertices(tess);
        const int* roofElements = tessGetElements(tess);
        const int roofVertexCount = tessGetVertexCount(tess);
        const int roofElementCount = tessGetElementCount(tess);
    
        // Reserve space for roof and sides
        _coords.reserve(roofVertexCount + edgeVertexCount * 6);
        _normals.reserve(roofVertexCount + edgeVertexCount * 6);
        
        // Calculate height in the internal coordinate system
        cglib::vec3<double> center = _boundingBox.center();
        float baseZ = static_cast<float>(center(2));
        float roofZ = static_cast<float>(center(2) + projection.toInternalScale(polygon3D.getHeight()));
        _boundingBox.add(cglib::vec3<double>(center(0), center(1), roofZ));
        
        // Convert triangulator output to coord array
        for (int i = 0; i < roofElementCount * MAX_INDICES_PER_ELEMENT; i += 3) {
            if (roofElements[i] != TESS_UNDEF && roofElements[i + 1] != TESS_UNDEF && roofElements[i + 2] != TESS_UNDEF) {
                unsigned int index = roofElements[i] * 2;
                _coords.emplace_back(roofCoords[index], roofCoords[index + 1], roofZ);
                index = roofElements[i + 1] * 2;
                _coords.emplace_back(roofCoords[index], roofCoords[index + 1], roofZ);
                index = roofElements[i + 2] * 2;
                _coords.emplace_back(roofCoords[index], roofCoords[index + 1], roofZ);
                
                cglib::vec3<float> roofNormal(0, 0, 1);
                _normals.push_back(roofNormal);
                _normals.push_back(roofNormal);
                _normals.push_back(roofNormal);
            }
        }
        tessDeleteTess(tess);
        
        // Calculate sides
        for (int i = -1; i < static_cast<int>(holes.size()); i++) {
            const std::vector<MapPos>* side = (i == -1 ? &poses : &holes[i]);
            const std::vector<double>* sideArray = (i == -1 ? &posesArray : &holesArray[i]);
            
            // Calculate vertex orientation of the polygon
            bool clockWise = GeomUtils::IsConcavePolygonClockwise(*side);
            // If calculating a whole, reverse the direction
            clockWise = (i >= 0) ? !clockWise : clockWise;
            
            const double* prevPos = sideArray->size() >= 2 ? &(*sideArray)[sideArray->size() - 2] : nullptr;
            for (std::size_t j = 0; j < sideArray->size(); j += 2) {
                const double* pos = &(*sideArray)[j];
                
                if (prevPos) {
                    const double* p0 = clockWise ? pos : prevPos;
                    const double* p1 = clockWise ? prevPos : pos;
                    
                    // Add coordinates for 2 triangles
                    _coords.emplace_back(p0[0], p0[1], roofZ);
                    _coords.emplace_back(p0[0], p0[1], baseZ);
                    _coords.emplace_back(p1[0], p1[1], roofZ);
                    _coords.emplace_back(p0[0], p0[1], baseZ);
                    _coords.emplace_back(p1[0], p1[1], baseZ);
                    _coords.emplace_back(p1[0], p1[1], roofZ);
                    
                    // Calculate side normal
                    cglib::vec3<float> sideNormal = cglib::unit(cglib::vec3<float>(static_cast<float>(p1[1] - p0[1]), static_cast<float>(p0[0] - p1[0]), 0));
                    
                    // Add normal for each vertex
                    _normals.push_back(sideNormal);
                    _normals.push_back(sideNormal);
                    _normals.push_back(sideNormal);
                    _normals.push_back(sideNormal);
                    _normals.push_back(sideNormal);
                    _normals.push_back(sideNormal);
                }
                prevPos = pos;
            }
        }
        
        if (_coords.size() > GLUtils::MAX_VERTEXBUFFER_SIZE) {
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
    
    void Polygon3DDrawData::offsetHorizontally(double offset) {
        for (cglib::vec3<double>& pos : _coords) {
            pos(0) += offset;
        }
        setIsOffset(true);
    }
    
}
