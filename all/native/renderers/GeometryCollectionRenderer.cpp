#include "GeometryCollectionRenderer.h"
#include "vectorelements/GeometryCollection.h"
#include "drawdatas/GeometryCollectionDrawData.h"
#include "drawdatas/LineDrawData.h"
#include "drawdatas/PointDrawData.h"
#include "drawdatas/PolygonDrawData.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "renderers/components/RayIntersectedElement.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Point.h"

#include <cglib/mat.h>

namespace carto {

    GeometryCollectionRenderer::GeometryCollectionRenderer() :
        _pointRenderer(),
        _lineRenderer(),
        _polygonRenderer(),
        _mutex()
    {
    }

    GeometryCollectionRenderer::~GeometryCollectionRenderer() {
    }

    void GeometryCollectionRenderer::offsetLayerHorizontally(double offset) {
        std::lock_guard<std::mutex> lock(_mutex);

        _pointRenderer.offsetLayerHorizontally(offset);
        _lineRenderer.offsetLayerHorizontally(offset);
        _polygonRenderer.offsetLayerHorizontally(offset);
    }

    void GeometryCollectionRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _pointRenderer.onSurfaceCreated(shaderManager, textureManager);
        _lineRenderer.onSurfaceCreated(shaderManager, textureManager);
        _polygonRenderer.onSurfaceCreated(shaderManager, textureManager);
    }

    void GeometryCollectionRenderer::onDrawFrame(float deltaSeconds, StyleTextureCache& styleCache, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const std::shared_ptr<GeometryCollection>& element : _elements) {
            for (const std::shared_ptr<VectorElementDrawData>& drawData : element->getDrawData()->getDrawDatas()) {
                if (std::shared_ptr<PointDrawData> pointDrawData = std::dynamic_pointer_cast<PointDrawData>(drawData)) {
                    if (!_lineRenderer.isEmptyBatch()) {
                        _lineRenderer.drawBatch(styleCache, viewState);
                        _lineRenderer.unbind();
                    } else if (!_polygonRenderer.isEmptyBatch()) {
                        _polygonRenderer.drawBatch(styleCache, viewState);
                        _polygonRenderer.unbind();
                    }
                    if (_pointRenderer.isEmptyBatch()) {
                        _pointRenderer.bind(viewState);
                    }
                    _pointRenderer.addToBatch(pointDrawData, styleCache, viewState);
                } else if (std::shared_ptr<LineDrawData> lineDrawData = std::dynamic_pointer_cast<LineDrawData>(drawData)) {
                    if (!_pointRenderer.isEmptyBatch()) {
                        _pointRenderer.drawBatch(styleCache, viewState);
                        _pointRenderer.unbind();
                    } else if (!_polygonRenderer.isEmptyBatch()) {
                        _polygonRenderer.drawBatch(styleCache, viewState);
                        _polygonRenderer.unbind();
                    }
                    if (_lineRenderer.isEmptyBatch()) {
                        _lineRenderer.bind(viewState);
                    }
                    _lineRenderer.addToBatch(lineDrawData, styleCache, viewState);
                } else if (std::shared_ptr<PolygonDrawData> polygonDrawData = std::dynamic_pointer_cast<PolygonDrawData>(drawData)) {
                    if (!_pointRenderer.isEmptyBatch()) {
                        _pointRenderer.drawBatch(styleCache, viewState);
                        _pointRenderer.unbind();
                    } else if (!_lineRenderer.isEmptyBatch()) {
                        _lineRenderer.drawBatch(styleCache, viewState);
                        _lineRenderer.unbind();
                    }
                    if (_polygonRenderer.isEmptyBatch()) {
                        _polygonRenderer.bind(viewState);
                    }
                    _polygonRenderer.addToBatch(polygonDrawData, styleCache, viewState);
                }
            }
        }

        if (!_pointRenderer._drawDataBuffer.empty()) {
            _pointRenderer.drawBatch(styleCache, viewState);
            _pointRenderer.unbind();
        }
        if (!_lineRenderer._drawDataBuffer.empty()) {
            _lineRenderer.drawBatch(styleCache, viewState);
            _lineRenderer.unbind();
        }
        if (!_polygonRenderer._drawDataBuffer.empty()) {
            _polygonRenderer.drawBatch(styleCache, viewState);
            _polygonRenderer.unbind();
        }
    }

    void GeometryCollectionRenderer::onSurfaceDestroyed() {
        _pointRenderer.onSurfaceDestroyed();
        _lineRenderer.onSurfaceDestroyed();
        _polygonRenderer.onSurfaceDestroyed();
    }

    void GeometryCollectionRenderer::addElement(const std::shared_ptr<GeometryCollection>& element) {
        _tempElements.push_back(element);
    }

    void GeometryCollectionRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }

    void GeometryCollectionRenderer::updateElement(const std::shared_ptr<GeometryCollection>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            _elements.push_back(element);
        }
    }

    void GeometryCollectionRenderer::removeElement(const std::shared_ptr<GeometryCollection>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
    }

    void GeometryCollectionRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const MapPos& rayOrig, const MapVec& rayDir, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const std::shared_ptr<GeometryCollection>& element : _elements) {
            for (const std::shared_ptr<VectorElementDrawData>& drawData : element->getDrawData()->getDrawDatas()) {
                if (std::shared_ptr<PointDrawData> pointDrawData = std::dynamic_pointer_cast<PointDrawData>(drawData)) {
                    PointRenderer::FindElementRayIntersection(element, pointDrawData, layer, rayOrig, rayDir, viewState, results);
                } else if (std::shared_ptr<LineDrawData> lineDrawData = std::dynamic_pointer_cast<LineDrawData>(drawData)) {
                    LineRenderer::FindElementRayIntersection(element, lineDrawData, layer, rayOrig, rayDir, viewState, results);
                } else if (std::shared_ptr<PolygonDrawData> polygonDrawData = std::dynamic_pointer_cast<PolygonDrawData>(drawData)) {
                    PolygonRenderer::FindElementRayIntersection(element, polygonDrawData, layer, rayOrig, rayDir, viewState, results);
                }
            }
        }
    }

}
