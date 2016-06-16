#include "NMLModelRenderer.h"
#include "datasources/VectorDataSource.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "projections/Projection.h"
#include "nml/Model.h"
#include "renderers/components/RayIntersectedElement.h"
#include "utils/Log.h"
#include "utils/GLES2.h"
#include "utils/GeomUtils.h"
#include "utils/GLUtils.h"

namespace carto {

    NMLModelRenderer::NMLModelRenderer() :
        _glModelMap(),
        _elements(),
        _tempElements()
    {
    }
    
    NMLModelRenderer::~NMLModelRenderer() {
    }
        
    void NMLModelRenderer::addElement(const std::shared_ptr<NMLModel>& element) {
        _tempElements.push_back(element);
    }
    
    void NMLModelRenderer::refreshElements() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.clear();
        _elements.swap(_tempElements);
    }
        
    void NMLModelRenderer::updateElement(const std::shared_ptr<NMLModel>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_elements.begin(), _elements.end(), element) == _elements.end()) {
            _elements.push_back(element);
        }
    }
    
    void NMLModelRenderer::removeElement(const std::shared_ptr<NMLModel>& element) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.erase(std::remove(_elements.begin(), _elements.end(), element), _elements.end());
    }
    
    void NMLModelRenderer::offsetLayerHorizontally(double offset) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }

    void NMLModelRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const MapPos& rayOrig, const MapVec& rayDir, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            const NMLModelDrawData& drawData = *element->getDrawData();
            
            std::shared_ptr<nml::Model> sourceModel = drawData.getSourceModel();
            GLModelMap::const_iterator model_it = _glModelMap.find(sourceModel);
            if (model_it == _glModelMap.end()) {
                continue;
            }
            std::shared_ptr<nmlgl::Model> glModel = model_it->second;
    
            cglib::mat4x4<double> modelMat = drawData.getLocalMat();
            cglib::mat4x4<double> invModelMat = cglib::inverse(modelMat);
    
            cglib::vec3<double> rayOrigModel = cglib::transform_point(cglib::vec3<double>(rayOrig.getX(), rayOrig.getY(), rayOrig.getZ()), invModelMat);
            cglib::vec3<double> rayDirModel = cglib::transform_point(cglib::vec3<double>(rayOrig.getX() + rayDir.getX(), rayOrig.getY() + rayDir.getY(), rayOrig.getZ() + rayDir.getZ()), invModelMat) - rayOrigModel;
            cglib::bbox3<float> modelBounds = glModel->getBounds();
            
            if (!GeomUtils::RayBoundingBoxIntersect(
                    MapPos(rayOrigModel(0), rayOrigModel(1), rayOrigModel(2)),
                    MapVec(rayDirModel(0), rayDirModel(1), rayDirModel(2)),
                    MapBounds(MapPos(modelBounds.min(0), modelBounds.min(1), modelBounds.min(2)), MapPos(modelBounds.max(0), modelBounds.max(1), modelBounds.max(2)))))
            {
                continue;
            }
            
            std::vector<nmlgl::RayIntersection> intersections;
            glModel->calculateRayIntersections(nmlgl::Ray(rayOrigModel, rayDirModel), intersections);
            
            for (size_t i = 0; i < intersections.size(); i++) {
                cglib::vec3<double> pos = cglib::transform_point(intersections[i].pos, modelMat);
                MapPos clickPos(pos(0), pos(1), pos(2));
                double distance = GeomUtils::DistanceFromPoint(clickPos, viewState.getCameraPos());
                MapPos projectedClickPos = layer->getDataSource()->getProjection()->fromInternal(clickPos);
                int priority = static_cast<int>(results.size());
                results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, projectedClickPos, projectedClickPos, priority));
            }
        }
    }
    
    bool NMLModelRenderer::drawModels(const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        // Draw all models, create missing model objects
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            const NMLModelDrawData& drawData = *element->getDrawData();
            std::shared_ptr<nml::Model> sourceModel = drawData.getSourceModel();
            std::shared_ptr<nmlgl::Model> glModel = _glModelMap[sourceModel];
            if (!glModel) {
                glModel = std::make_shared<nmlgl::Model>(*sourceModel);
                glModel->create(_glContext);
                _glModelMap[sourceModel] = glModel;
            }
    
            cglib::mat4x4<float> lmvpMat = cglib::mat4x4<float>::convert(viewState.getModelviewProjectionMat() * drawData.getLocalMat());
            _glContext->setModelviewProjectionMatrix(lmvpMat);
    
            glModel->draw(_glContext);
        }
    
        // Dispose unused models
        for (GLModelMap::iterator it = _glModelMap.begin(); it != _glModelMap.end(); ) {
            if (it->first.unique()) {
                it->second->dispose(_glContext);
                it = _glModelMap.erase(it);
            } else {
                it++;
            }
        }
        
        // No need to refresh
        return false;
    }
    
}
