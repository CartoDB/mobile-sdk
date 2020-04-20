#include "PolygonDrawData.h"
#include "core/MapPos.h"
#include "geometry/PolygonGeometry.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "renderers/utils/GLContext.h"
#include "styles/PolygonStyle.h"
#include "vectorelements/Line.h"
#include "vectorelements/Polygon.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cmath>
#include <cstdlib>
#include <unordered_map>

#include <tesselator.h>

namespace carto {

    PolygonDrawData::PolygonDrawData(const PolygonGeometry& geometry, const PolygonStyle& style, const Projection& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface) :
        VectorElementDrawData(style.getColor(), projectionSurface),
        _bitmap(style.getBitmap()),
        _boundingBox(cglib::bbox3<double>::smallest()),
        _coords(),
        _indices(),
        _lineDrawDatas()
    {
        const std::vector<MapPos>& poses = geometry.getPoses();
        const std::vector<std::vector<MapPos> >& holes = geometry.getHoles();
        
        // Create tesselator
        TESSalloc ma;
        ma.memalloc = [](void* userData, unsigned int size) { return malloc(size); };
        ma.memfree = [](void* userData, void* ptr) { free(ptr); };
        ma.extraVertices = 256;
        TESStesselator* tessPtr = tessNewTess(&ma);
        if (!tessPtr) {
            Log::Error("PolygonDrawData::PolygonDrawData: Failed to create tesselator!");
            return;
        }
        std::shared_ptr<TESStesselator> tess(tessPtr, tessDeleteTess);

        // Add polygon exterior, calculate bounding box
        std::vector<double> posesArray(poses.size() * 3);
        std::vector<MapPos> ringPoses;
        ringPoses.reserve(poses.size() + 1);
        for (std::size_t i = 0; i < poses.size(); i++) {
            MapPos internalPos = projection.toInternal(poses[i]);
            posesArray[i * 3 + 0] = internalPos.getX();
            posesArray[i * 3 + 1] = internalPos.getY();
            posesArray[i * 3 + 2] = internalPos.getZ();
            ringPoses.push_back(poses[i]);
        }
        tessAddContour(tess.get(), 3, posesArray.data(), sizeof(double) * 3, static_cast<unsigned int>(poses.size()));
    
        if (style.getLineStyle() && !poses.empty()) {
            ringPoses.push_back(poses.front());
            _lineDrawDatas.push_back(std::make_shared<LineDrawData>(ringPoses, *style.getLineStyle(), projection, projectionSurface));
        }
    
        // Add polygon holes
        for (const std::vector<MapPos>& hole : holes) {
            ringPoses.clear();
            ringPoses.reserve(hole.size() + 1);
            std::vector<double> holeArray(hole.size() * 3);
            for (std::size_t i = 0; i < hole.size(); i++) {
                MapPos internalPos = projection.toInternal(hole[i]);
                holeArray[i * 3 + 0] = internalPos.getX();
                holeArray[i * 3 + 1] = internalPos.getY();
                holeArray[i * 3 + 2] = internalPos.getZ();
                ringPoses.push_back(hole[i]);
            }
            tessAddContour(tess.get(), 3, holeArray.data(), sizeof(double) * 3, static_cast<unsigned int>(hole.size()));
    
            if (style.getLineStyle() && !hole.empty()) {
                ringPoses.push_back(hole.front());
                _lineDrawDatas.push_back(std::make_shared<LineDrawData>(ringPoses, *style.getLineStyle(), projection, projectionSurface));
            }
        }
    
        // Triangulate
        if (!tessTesselate(tess.get(), TESS_WINDING_ODD, TESS_POLYGONS, 3, 3, NULL)) {
            Log::Error("PolygonDrawData::PolygonDrawData: Failed to triangulate polygon!");
            return;
        }
        const double* coords = tessGetVertices(tess.get());
        const int* elements = tessGetElements(tess.get());
        std::size_t vertexCount = tessGetVertexCount(tess.get());
        std::size_t elementCount = tessGetElementCount(tess.get());

        // Do projection-surface based tesselation
        std::vector<MapPos> internalPoses;
        internalPoses.reserve(vertexCount);
        for (std::size_t i = 0; i < vertexCount; i++) {
            internalPoses.emplace_back(coords[i * 3 + 0], coords[i * 3 + 1], coords[i * 3 + 2]);
        }
        std::vector<unsigned int> indices;
        indices.reserve(elementCount * 3);
        for (std::size_t i = 0; i < elementCount * 3; i += 3) {
            unsigned int i0 = elements[i + 0];
            unsigned int i1 = elements[i + 1];
            unsigned int i2 = elements[i + 2];
            if (i0 != TESS_UNDEF && i1 != TESS_UNDEF && i2 != TESS_UNDEF) {
                projectionSurface->tesselateTriangle(i0, i1, i2, indices, internalPoses);
            }
        }
    
        // Convert tesselation results to drawable format, split if into multiple buffers, if the polyong is too big
        _coords.push_back(std::vector<cglib::vec3<double> >());
        _coords.back().reserve(std::min(internalPoses.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
        _indices.push_back(std::vector<unsigned int>());
        _indices.back().reserve(std::min(indices.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
        std::unordered_map<unsigned int, unsigned int> indexMap;
        indexMap.reserve(std::min(indices.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
        for (std::size_t i = 0; i < indices.size(); i += 3) {
            // Check for possible GL buffer overflow
            if (_indices.back().size() + 3 > GLContext::MAX_VERTEXBUFFER_SIZE) {
                // The buffer is full, create a new one
                _coords.back().shrink_to_fit();
                _coords.push_back(std::vector<cglib::vec3<double> >());
                _coords.back().reserve(std::min(internalPoses.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
                _indices.back().shrink_to_fit();
                _indices.push_back(std::vector<unsigned int>());
                _indices.back().reserve(std::min(indices.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
                indexMap.clear();
            }
            
            for (int j = 0; j < 3; j++) {
                unsigned int index = static_cast<unsigned int>(indices[i + j]);
                auto it = indexMap.find(index);
                if (it == indexMap.end()) {
                    unsigned int newIndex = static_cast<unsigned int>(_coords.back().size());
                    _coords.back().push_back(projectionSurface->calculatePosition(internalPoses[index]));
                    _boundingBox.add(_coords.back().back());
                    _indices.back().push_back(newIndex);
                    indexMap[index] = newIndex;
                } else {
                    _indices.back().push_back(it->second);
                }
            }
        }

        _coords.back().shrink_to_fit();
        _indices.back().shrink_to_fit();
    }
    
    PolygonDrawData::~PolygonDrawData() {
    }
    
    const std::shared_ptr<Bitmap> PolygonDrawData::getBitmap() const {
        return _bitmap;
    }
    
    const cglib::bbox3<double>& PolygonDrawData::getBoundingBox() const {
        return _boundingBox;
    }
    
    const std::vector<std::vector<cglib::vec3<double> > >& PolygonDrawData::getCoords() const {
        return _coords;
    }
    
    const std::vector<std::vector<unsigned int> >& PolygonDrawData::getIndices() const {
        return _indices;
    }
    
    const std::vector<std::shared_ptr<LineDrawData> >& PolygonDrawData::getLineDrawDatas() const {
        return _lineDrawDatas;
    }
    
    void PolygonDrawData::offsetHorizontally(double offset) {
        for (std::vector<cglib::vec3<double> >& coords : _coords) {
            for (cglib::vec3<double>& coord : coords) {
                coord(0) += offset;
            }
        }
    
        for (const std::shared_ptr<LineDrawData>& drawData : _lineDrawDatas) {
            drawData->offsetHorizontally(offset);
        }
        
        setIsOffset(true);
    }
    
}
