#include "PolygonDrawData.h"
#include "core/MapPos.h"
#include "geometry/PolygonGeometry.h"
#include "projections/Projection.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "styles/PolygonStyle.h"
#include "vectorelements/Line.h"
#include "vectorelements/Polygon.h"
#include "utils/Const.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"

#include <cmath>
#include <cstdlib>
#include <tesselator.h>
#include <unordered_map>

namespace {

    void* polygonStdAlloc(void* userData, unsigned int size) {
        return malloc(size);
    }
    
    void polygonStdFree(void* userData, void* ptr) {
        free(ptr);
    }
    
}

namespace carto {

    PolygonDrawData::PolygonDrawData(const PolygonGeometry& geometry, const PolygonStyle& style, const Projection& projection) :
        VectorElementDrawData(style.getColor()),
        _bitmap(style.getBitmap()),
        _boundingBox(),
        _coords(),
        _indices(),
        _lineDrawDatas()
    {
        // Reserve space for line draw datas
        int lineDrawDataCount = 1;
        if (style.getLineStyle()) {
            lineDrawDataCount += geometry.getHoles().size();
        }
        _lineDrawDatas.reserve(lineDrawDataCount);
        
        TESSalloc ma;
        ma.memalloc = polygonStdAlloc;
        ma.memfree = polygonStdFree;
        ma.extraVertices = 256;
        TESStesselator* tess = tessNewTess(&ma);
    
        // Add polygon exterior, calculate bounding box
        const std::vector<MapPos>& poses = geometry.getPoses();
        std::vector<MapPos> internalPoses;
        internalPoses.reserve(poses.size());
        std::vector<double> posesArray(poses.size() * 2);
        for (std::size_t i = 0; i < poses.size() * 2; i += 2) {
            std::size_t index = i / 2;
            internalPoses.push_back(projection.toInternal(poses[index]));
            posesArray[i] = internalPoses.back().getX();
            posesArray[i + 1] = internalPoses.back().getY();
            _boundingBox.expandToContain(internalPoses.back());
        }
        tessAddContour(tess, 2, &posesArray[0], sizeof(double) * 2, static_cast<unsigned int>(poses.size()));
    
        if (style.getLineStyle()) {
            _lineDrawDatas.emplace_back(geometry, internalPoses, *style.getLineStyle(), projection);
        }
    
        // Add polygon holes
        const std::vector<std::vector<MapPos> >& holes = geometry.getHoles();
        for (const std::vector<MapPos>& hole : holes) {
            internalPoses.clear();
            internalPoses.reserve(hole.size());
            std::vector<double> holeArray(hole.size() * 2);
            for (std::size_t i = 0; i < hole.size() * 2; i += 2) {
                std::size_t index = i / 2;
                internalPoses.push_back(projection.toInternal(hole[index]));
                holeArray[i] = internalPoses.back().getX();
                holeArray[i + 1] = internalPoses.back().getY();
                _boundingBox.expandToContain(internalPoses.back());
            }
            tessAddContour(tess, 2, &holeArray[0], sizeof(double) * 2, static_cast<unsigned int>(hole.size()));
    
            if (style.getLineStyle()) {
                _lineDrawDatas.emplace_back(geometry, internalPoses, *style.getLineStyle(), projection);
            }
        }
    
        // Tesselate
        double normal[3];
        normal[0] = 0;
        normal[1] = 0;
        normal[2] = 1;
        if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, MAX_INDICES_PER_ELEMENT, 2, normal)) {
            Log::Error("PolygonDrawData::PolygonDrawData: Failed to triangulate polygon!");
            tessDeleteTess(tess);
            return;
        }
    
        // Get tesselation results
        const double* coords = tessGetVertices(tess);
        const int* elements = tessGetElements(tess);
        size_t vertexCount = tessGetVertexCount(tess);
        size_t elementCount = tessGetElementCount(tess);
    
        // Calculate center
        const MapPos& center = _boundingBox.getCenter();
    
        // Convert tesselation results to drawable format, split if into multiple buffers, if the polyong is too big
        _coords.push_back(std::vector<MapPos>());
        _coords.back().reserve(std::min(vertexCount, GLUtils::MAX_VERTEXBUFFER_SIZE));
        _indices.push_back(std::vector<unsigned int>());
        _indices.back().reserve(std::min(elementCount * MAX_INDICES_PER_ELEMENT, GLUtils::MAX_VERTEXBUFFER_SIZE));
        std::unordered_map<unsigned int, unsigned int> indexMap;
        for (size_t i = 0; i < elementCount * MAX_INDICES_PER_ELEMENT; i += 3) {
            
            // Check for possible GL buffer overflow
            if (_indices.back().size() + 3 > GLUtils::MAX_VERTEXBUFFER_SIZE) {
                // The buffer is full, create a new one
                _coords.back().shrink_to_fit();
                _coords.push_back(std::vector<MapPos>());
                _coords.back().reserve(std::min(vertexCount, GLUtils::MAX_VERTEXBUFFER_SIZE));
                _indices.back().shrink_to_fit();
                _indices.push_back(std::vector<unsigned int>());
                _indices.back().reserve(std::min(elementCount * MAX_INDICES_PER_ELEMENT, GLUtils::MAX_VERTEXBUFFER_SIZE));
                indexMap.clear();
            }
            
            if (elements[i] == TESS_UNDEF || elements[i + 1] == TESS_UNDEF || elements[i + 2] == TESS_UNDEF) {
                _coords.clear();
                _indices.clear();
                Log::Error("PolygonDrawData::PolygonDrawData: Undefined element in tessellation");
                break;
            }
            
            for (int j = 0; j < 3; j++) {
                unsigned int index = static_cast<unsigned int>(elements[i + j]);
                auto it = indexMap.find(index);
                if (it == indexMap.end()) {
                    unsigned int newIndex = static_cast<unsigned int>(_coords.back().size());
                    _coords.back().emplace_back(coords[index * 2], coords[index * 2 + 1], center.getZ());
                    _indices.back().push_back(newIndex);
                    indexMap[index] = newIndex;
                } else {
                    _indices.back().push_back(it->second);
                }
            }
        }
        
        _coords.back().shrink_to_fit();
        _indices.back().shrink_to_fit();
        
        tessDeleteTess(tess);
    }
    
    PolygonDrawData::~PolygonDrawData() {
    }
    
    const std::shared_ptr<Bitmap> PolygonDrawData::getBitmap() const {
        return _bitmap;
    }
    
    const MapBounds& PolygonDrawData::getBoundingBox() const {
        return _boundingBox;
    }
    
    const std::vector<std::vector<MapPos> >& PolygonDrawData::getCoords() const {
        return _coords;
    }
    
    const std::vector<std::vector<unsigned int> >& PolygonDrawData::getIndices() const {
        return _indices;
    }
    
    const std::vector<LineDrawData>& PolygonDrawData::getLineDrawDatas() const {
        return _lineDrawDatas;
    }
    
    void PolygonDrawData::offsetHorizontally(double offset) {
        for (std::vector<MapPos>& coords : _coords) {
            for (MapPos& coord : coords) {
                coord.setX(coord.getX() + offset);
            }
        }
    
        for (LineDrawData& drawData : _lineDrawDatas) {
            drawData.offsetHorizontally(offset);
        }
        
        setIsOffset(true);
    }
    
}
