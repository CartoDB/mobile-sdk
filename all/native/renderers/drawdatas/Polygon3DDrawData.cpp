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
#include <tesselator.h>

namespace carto {

    Polygon3DDrawData::Polygon3DDrawData(const Polygon3D& polygon3D, const Polygon3DStyle& style, const Projection& projection, const ProjectionSurface& projectionSurface) :
        VectorElementDrawData(style.getColor()),
        _sideColor(GetPremultipliedColor(style.getSideColor())),
        _boundingBox(cglib::bbox3<double>::smallest()),
        _coords(),
        _normals()
    {
        // Triangulate polygon roof
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
    
        // Prepare polygon exterior, calculate bounding box
        const std::vector<MapPos>& poses = polygon3D.getGeometry()->getPoses();
        std::size_t edgeVertexCount = poses.size();
        // TODO: make it 3D
        std::vector<double> posesArray(poses.size() * 2);
        for (std::size_t i = 0; i < poses.size(); i++) {
            MapPos internalPos = projection.toInternal(poses[i]);
            posesArray[i * 2 + 0] = internalPos.getX();
            posesArray[i * 2 + 1] = internalPos.getY();
        }
        tessAddContour(tess.get(), 2, posesArray.data(), sizeof(double) * 2, static_cast<unsigned int>(poses.size()));
    
        // Prepare polygon holes
        const std::vector<std::vector<MapPos> >& holes = polygon3D.getGeometry()->getHoles();
        std::vector<std::vector<double> > holesArray(holes.size() * 2);
        for (std::size_t n = 0; n < holes.size(); n++) {
            const std::vector<MapPos>& hole = holes[n];
            edgeVertexCount += hole.size();
            std::vector<double>& holeArray = holesArray[n];
            holeArray.resize(hole.size() * 2);
            for (std::size_t i = 0; i < hole.size(); i++) {
                MapPos internalPos = projection.toInternal(hole[i]);
                holeArray[i * 2 + 0] = internalPos.getX();
                holeArray[i * 2 + 1] = internalPos.getY();
            }
            tessAddContour(tess.get(), 2, holeArray.data(), sizeof(double) * 2, static_cast<unsigned int>(hole.size()));
        }
    
        // Triangulate
        // TODO: normal
        double normal[3] = { 0, 0, 1 };
        if (!tessTesselate(tess.get(), TESS_WINDING_ODD, TESS_POLYGONS, MAX_INDICES_PER_ELEMENT, 2, normal)) {
            Log::Error("Polygon3DDrawData::Polygon3DDrawData: Failed to triangulate 3d polygon!");
            return;
        }
    
        const double* roofCoords = tessGetVertices(tess.get());
        const int* roofElements = tessGetElements(tess.get());
        const int roofVertexCount = tessGetVertexCount(tess.get());
        const int roofElementCount = tessGetElementCount(tess.get());
    
        // Reserve space for roof and sides
        _coords.reserve(roofVertexCount + edgeVertexCount * 6);
        _normals.reserve(roofVertexCount + edgeVertexCount * 6);
        
        // Calculate height in the internal coordinate system
        float baseZ = 0;
        float roofZ = projection.toInternalScale(polygon3D.getHeight());
        
        // Convert triangulator output to coord array
        for (int i = 0; i < roofElementCount * MAX_INDICES_PER_ELEMENT; i += 3) {
            if (roofElements[i + 0] != TESS_UNDEF && roofElements[i + 1] != TESS_UNDEF && roofElements[i + 2] != TESS_UNDEF) {
                for (int j = 0; j < 3; j++) {
                    unsigned int index = roofElements[i + j];
                    MapPos internalPos(roofCoords[index * 2 + 0], roofCoords[index * 2 + 1], roofZ);
                    _coords.push_back(projectionSurface.calculatePosition(internalPos));
                    _normals.push_back(cglib::vec3<float>::convert(projectionSurface.calculateNormal(internalPos)));
                }
            }
        }
        
        // Calculate sides
        for (int i = -1; i < static_cast<int>(holes.size()); i++) {
            const std::vector<MapPos>* side = (i == -1 ? &poses : &holes[i]);
            const std::vector<double>* sideArray = (i == -1 ? &posesArray : &holesArray[i]);

            // TODO: test should be done based on internal coordinates
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

                    MapPos internalPos0(p0[0], p0[1], roofZ);
                    MapPos internalPos1(p0[0], p0[1], baseZ);
                    MapPos internalPos2(p1[0], p1[1], roofZ);
                    MapPos internalPos3(p0[0], p0[1], baseZ);
                    MapPos internalPos4(p1[0], p1[1], baseZ);
                    MapPos internalPos5(p1[0], p1[1], roofZ);

                    // Add coordinates for 2 triangles
                    _coords.push_back(projectionSurface.calculatePosition(internalPos0));
                    _boundingBox.add(_coords.back());
                    _coords.push_back(projectionSurface.calculatePosition(internalPos1));
                    _boundingBox.add(_coords.back());
                    _coords.push_back(projectionSurface.calculatePosition(internalPos2));
                    _boundingBox.add(_coords.back());
                    _coords.push_back(projectionSurface.calculatePosition(internalPos3));
                    _boundingBox.add(_coords.back());
                    _coords.push_back(projectionSurface.calculatePosition(internalPos4));
                    _boundingBox.add(_coords.back());
                    _coords.push_back(projectionSurface.calculatePosition(internalPos5));
                    _boundingBox.add(_coords.back());

                    // Calculate side normal
                    cglib::vec3<float> sideNormal = cglib::unit(cglib::vec3<float>::convert(projectionSurface.calculateVector(internalPos0, MapVec(p1[1] - p0[1], p0[0] - p1[0], 0))));
                    
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
    
    void Polygon3DDrawData::offsetHorizontally(double offset) {
        for (cglib::vec3<double>& pos : _coords) {
            pos(0) += offset;
        }
        setIsOffset(true);
    }
    
}
