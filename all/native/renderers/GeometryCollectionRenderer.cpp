#include "GeometryCollectionRenderer.h"
#include "vectorelements/GeometryCollection.h"
#include "drawdatas/GeometryCollectionDrawData.h"
#include "drawdatas/LineDrawData.h"
#include "drawdatas/PointDrawData.h"
#include "drawdatas/PolygonDrawData.h"
#include "graphics/ViewState.h"
#include "renderers/components/RayIntersectedElement.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "vectorelements/Point.h"

#include <cglib/mat.h>

namespace carto {

    GeometryCollectionRenderer::GeometryCollectionRenderer() :
        _elements(),
        _tempElements(),
        _pointRenderer(),
        _lineRenderer(),
        _polygonRenderer(),
        _mutex()
    {
    }

    GeometryCollectionRenderer::~GeometryCollectionRenderer() {
    }

    void GeometryCollectionRenderer::setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer) {
        std::lock_guard<std::mutex> lock(_mutex);

        _pointRenderer.setComponents(options, mapRenderer);
        _lineRenderer.setComponents(options, mapRenderer);
        _polygonRenderer.setComponents(options, mapRenderer);
    }

    void GeometryCollectionRenderer::offsetLayerHorizontally(double offset) {
        std::lock_guard<std::mutex> lock(_mutex);

        _pointRenderer.offsetLayerHorizontally(offset);
        _lineRenderer.offsetLayerHorizontally(offset);
        _polygonRenderer.offsetLayerHorizontally(offset);
    }

    void GeometryCollectionRenderer::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_elements.empty()) {
            // Early return, to avoid calling glUseProgram etc.
            return;
        }

        if (!initializeRenderer()) {
            return;
        }

        glDisable(GL_CULL_FACE);

        for (const std::shared_ptr<GeometryCollection>& element : _elements) {
            for (const std::shared_ptr<VectorElementDrawData>& drawData : element->getDrawData()->getDrawDatas()) {
                if (std::shared_ptr<PointDrawData> pointDrawData = std::dynamic_pointer_cast<PointDrawData>(drawData)) {
                    if (!_lineRenderer.isEmptyBatch()) {
                        _lineRenderer.drawBatch(viewState);
                        _lineRenderer.unbind();
                    } else if (!_polygonRenderer.isEmptyBatch()) {
                        _polygonRenderer.drawBatch(viewState);
                        _polygonRenderer.unbind();
                    }
                    if (_pointRenderer.isEmptyBatch()) {
                        _pointRenderer.bind(viewState);
                    }
                    _pointRenderer.addToBatch(pointDrawData, viewState);
                } else if (std::shared_ptr<LineDrawData> lineDrawData = std::dynamic_pointer_cast<LineDrawData>(drawData)) {
                    if (!_pointRenderer.isEmptyBatch()) {
                        _pointRenderer.drawBatch(viewState);
                        _pointRenderer.unbind();
                    } else if (!_polygonRenderer.isEmptyBatch()) {
                        _polygonRenderer.drawBatch(viewState);
                        _polygonRenderer.unbind();
                    }
                    if (_lineRenderer.isEmptyBatch()) {
                        _lineRenderer.bind(viewState);
                    }
                    _lineRenderer.addToBatch(lineDrawData, viewState);
                } else if (std::shared_ptr<PolygonDrawData> polygonDrawData = std::dynamic_pointer_cast<PolygonDrawData>(drawData)) {
                    if (!_pointRenderer.isEmptyBatch()) {
                        _pointRenderer.drawBatch(viewState);
                        _pointRenderer.unbind();
                    } else if (!_lineRenderer.isEmptyBatch()) {
                        _lineRenderer.drawBatch(viewState);
                        _lineRenderer.unbind();
                    }
                    if (_polygonRenderer.isEmptyBatch()) {
                        _polygonRenderer.bind(viewState);
                    }
                    _polygonRenderer.addToBatch(polygonDrawData, viewState);
                }
            }
        }

        if (!_pointRenderer._drawDataBuffer.empty()) {
            _pointRenderer.drawBatch(viewState);
            _pointRenderer.unbind();
        }
        if (!_lineRenderer._drawDataBuffer.empty()) {
            _lineRenderer.drawBatch(viewState);
            _lineRenderer.unbind();
        }
        if (!_polygonRenderer._drawDataBuffer.empty()) {
            _polygonRenderer.drawBatch(viewState);
            _polygonRenderer.unbind();
        }
        
        glEnable(GL_CULL_FACE);
    }

    void GeometryCollectionRenderer::addElement(const std::shared_ptr<GeometryCollection>& element) {
        if (element->getDrawData()) {
            _tempElements.push_back(element);
        }
    }

    void GeometryCollectionRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }

    void GeometryCollectionRenderer::updateElement(const std::shared_ptr<GeometryCollection>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            if (element->getDrawData()) {
                _elements.push_back(element);
            }
        }
    }

    void GeometryCollectionRenderer::removeElement(const std::shared_ptr<GeometryCollection>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
    }

    void GeometryCollectionRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);

        for (const std::shared_ptr<GeometryCollection>& element : _elements) {
            for (const std::shared_ptr<VectorElementDrawData>& drawData : element->getDrawData()->getDrawDatas()) {
                if (std::shared_ptr<PointDrawData> pointDrawData = std::dynamic_pointer_cast<PointDrawData>(drawData)) {
                    PointRenderer::FindElementRayIntersection(element, pointDrawData, layer, ray, viewState, results);
                } else if (std::shared_ptr<LineDrawData> lineDrawData = std::dynamic_pointer_cast<LineDrawData>(drawData)) {
                    LineRenderer::FindElementRayIntersection(element, lineDrawData, layer, ray, viewState, results);
                } else if (std::shared_ptr<PolygonDrawData> polygonDrawData = std::dynamic_pointer_cast<PolygonDrawData>(drawData)) {
                    PolygonRenderer::FindElementRayIntersection(element, polygonDrawData, layer, ray, viewState, results);
                }
            }
        }
    }

    bool GeometryCollectionRenderer::initializeRenderer() {
        return _pointRenderer.initializeRenderer() && _lineRenderer.initializeRenderer() && _polygonRenderer.initializeRenderer();
    }

}
