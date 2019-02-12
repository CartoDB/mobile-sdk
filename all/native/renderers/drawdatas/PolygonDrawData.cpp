#include "PolygonDrawData.h"
#include "core/MapPos.h"
#include "geometry/PolygonGeometry.h"
#include "graphics/utils/GLContext.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "styles/PolygonStyle.h"
#include "vectorelements/Line.h"
#include "vectorelements/Polygon.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cmath>
#include <cstdlib>
#include <tesselator.h>
#include <unordered_map>

namespace carto {

    PolygonDrawData::PolygonDrawData(const PolygonGeometry& geometry, const PolygonStyle& style, const Projection& projection, const ProjectionSurface& projectionSurface) :
        VectorElementDrawData(style.getColor()),
        _bitmap(style.getBitmap()),
        _boundingBox(cglib::bbox3<double>::smallest()),
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
        const std::vector<MapPos>& poses = geometry.getPoses();
        std::vector<double> posesArray(poses.size() * 2);
        std::vector<MapPos> linePoses;
        linePoses.reserve(poses.size() + 1);
        for (std::size_t i = 0; i < poses.size(); i++) {
            MapPos internalPos = projection.toInternal(poses[i]);
            posesArray[i * 2 + 0] = internalPos.getX();
            posesArray[i * 2 + 1] = internalPos.getY();
            linePoses.push_back(poses[i]);
        }
        tessAddContour(tess.get(), 2, posesArray.data(), sizeof(double) * 2, static_cast<unsigned int>(poses.size()));
    
        if (style.getLineStyle() && !poses.empty()) {
            linePoses.push_back(poses.front());
            _lineDrawDatas.push_back(std::make_shared<LineDrawData>(linePoses, *style.getLineStyle(), projection, projectionSurface));
        }
    
        // Add polygon holes
        const std::vector<std::vector<MapPos> >& holes = geometry.getHoles();
        for (const std::vector<MapPos>& hole : holes) {
            linePoses.clear();
            linePoses.reserve(hole.size() + 1);
            std::vector<double> holeArray(hole.size() * 2);
            for (std::size_t i = 0; i < hole.size(); i++) {
                MapPos internalPos = projection.toInternal(hole[i]);
                holeArray[i * 2 + 0] = internalPos.getX();
                holeArray[i * 2 + 1] = internalPos.getY();
                linePoses.push_back(hole[i]);
            }
            tessAddContour(tess.get(), 2, holeArray.data(), sizeof(double) * 2, static_cast<unsigned int>(hole.size()));
    
            if (style.getLineStyle() && !hole.empty()) {
                linePoses.push_back(hole.front());
                _lineDrawDatas.push_back(std::make_shared<LineDrawData>(linePoses, *style.getLineStyle(), projection, projectionSurface));
            }
        }
    
        // Tesselate
        // TODO: normal
        double normal[3] = { 0, 0, 1 };
        if (!tessTesselate(tess.get(), TESS_WINDING_ODD, TESS_POLYGONS, MAX_INDICES_PER_ELEMENT, 2, normal)) {
            Log::Error("PolygonDrawData::PolygonDrawData: Failed to triangulate polygon!");
            return;
        }
    
        // Get tesselation results
        const double* coords = tessGetVertices(tess.get());
        const int* elements = tessGetElements(tess.get());
        std::size_t vertexCount = tessGetVertexCount(tess.get());
        std::size_t elementCount = tessGetElementCount(tess.get());
    
        // Convert tesselation results to drawable format, split if into multiple buffers, if the polyong is too big
        _coords.push_back(std::vector<cglib::vec3<double> >());
        _coords.back().reserve(std::min(vertexCount, GLContext::MAX_VERTEXBUFFER_SIZE));
        _indices.push_back(std::vector<unsigned int>());
        _indices.back().reserve(std::min(elementCount * MAX_INDICES_PER_ELEMENT, GLContext::MAX_VERTEXBUFFER_SIZE));
        std::unordered_map<unsigned int, unsigned int> indexMap;
        for (std::size_t i = 0; i < elementCount * MAX_INDICES_PER_ELEMENT; i += 3) {
            
            // Check for possible GL buffer overflow
            if (_indices.back().size() + 3 > GLContext::MAX_VERTEXBUFFER_SIZE) {
                // The buffer is full, create a new one
                _coords.back().shrink_to_fit();
                _coords.push_back(std::vector<cglib::vec3<double> >());
                _coords.back().reserve(std::min(vertexCount, GLContext::MAX_VERTEXBUFFER_SIZE));
                _indices.back().shrink_to_fit();
                _indices.push_back(std::vector<unsigned int>());
                _indices.back().reserve(std::min(elementCount * MAX_INDICES_PER_ELEMENT, GLContext::MAX_VERTEXBUFFER_SIZE));
                indexMap.clear();
            }
            
            if (elements[i + 0] != TESS_UNDEF && elements[i + 1] != TESS_UNDEF && elements[i + 2] != TESS_UNDEF) {
                for (int j = 0; j < 3; j++) {
                    unsigned int index = static_cast<unsigned int>(elements[i + j]);
                    auto it = indexMap.find(index);
                    if (it == indexMap.end()) {
                        unsigned int newIndex = static_cast<unsigned int>(_coords.back().size());
                        MapPos posInternal(coords[index * 2 + 0], coords[index * 2 + 1], 0);
                        _coords.back().push_back(projectionSurface.calculatePosition(posInternal));
                        _boundingBox.add(_coords.back().back());
                        _indices.back().push_back(newIndex);
                        indexMap[index] = newIndex;
                    }
                    else {
                        _indices.back().push_back(it->second);
                    }
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
