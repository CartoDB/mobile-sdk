#include "NMLModelRenderer.h"
#include "components/ThreadWorker.h"
#include "datasources/VectorDataSource.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "projections/ProjectionSurface.h"
#include "renderers/MapRenderer.h"
#include "renderers/drawdatas/NMLModelDrawData.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/NMLResources.h"
#include "vectorelements/NMLModel.h"
#include "utils/Log.h"

#include <nml/GLModel.h>
#include <nml/GLTexture.h>
#include <nml/GLResourceManager.h>
#include <nml/Package.h>

namespace carto {

    NMLModelRenderer::NMLModelRenderer() :
        _mapRenderer(),
        _options(),
        _nmlResources(),
        _nmlModelMap(),
        _elements(),
        _tempElements(),
        _mutex()
    {
    }
    
    NMLModelRenderer::~NMLModelRenderer() {
    }
        
    void NMLModelRenderer::setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer) {
        std::lock_guard<std::mutex> lock(_mutex);

        _options = options;
        _mapRenderer = mapRenderer;
        _nmlResources.reset();
        _nmlModelMap.clear();
    }

    void NMLModelRenderer::offsetLayerHorizontally(double offset) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }

    bool NMLModelRenderer::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);

        std::shared_ptr<Options> options = _options.lock();
        if (!options) {
            return false;
        }

        if (_elements.empty()) {
            // Early return, to avoid calling glUseProgram etc.
            return false;
        }

        if (!initializeRenderer()) {
            return false;
        }

        std::shared_ptr<nml::GLResourceManager> resourceManager = _nmlResources->getResourceManager();
        if (!resourceManager) {
            return false;
        }

        // Set expected GL state
        glDepthMask(GL_TRUE);

        // Calculate lighting state
        Color optionsAmbientLightColor = options->getAmbientLightColor();
        cglib::vec4<float> ambientLightColor = cglib::vec4<float>(optionsAmbientLightColor.getR(), optionsAmbientLightColor.getG(), optionsAmbientLightColor.getB(), optionsAmbientLightColor.getA()) * (1.0f / 255.0f);
        Color optionsMainLightColor = options->getMainLightColor();
        cglib::vec4<float> mainLightColor = cglib::vec4<float>(optionsMainLightColor.getR(), optionsMainLightColor.getG(), optionsMainLightColor.getB(), optionsMainLightColor.getA()) * (1.0f / 255.0f);
        MapVec optionsMainLightDirection = options->getMainLightDirection();
        MapPos internalFocusPos = viewState.getProjectionSurface()->calculateMapPos(viewState.getFocusPos());
        cglib::vec3<float> mainLightDir = cglib::vec3<float>::convert(cglib::unit(viewState.getProjectionSurface()->calculateVector(internalFocusPos, optionsMainLightDirection)));

        // Draw models
        cglib::mat4x4<float> projMat = cglib::mat4x4<float>::convert(viewState.getProjectionMat());
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            const NMLModelDrawData& drawData = *element->getDrawData();

            Color drawDataColor = drawData.getColor();
            cglib::vec4<float> modelColor = cglib::vec4<float>(drawDataColor.getR(), drawDataColor.getG(), drawDataColor.getB(), drawDataColor.getA()) * (1.0f / 255.0f);
            std::shared_ptr<nml::Model> sourceModel = drawData.getSourceModel();
            std::shared_ptr<nml::GLModel> glModel = _nmlModelMap[sourceModel];
            if (!glModel) {
                glModel = std::make_shared<nml::GLModel>(*sourceModel);
                glModel->create(*resourceManager);
                _nmlModelMap[sourceModel] = glModel;
            }
    
            cglib::mat4x4<float> mvMat = cglib::mat4x4<float>::convert(viewState.getModelviewMat() * drawData.getLocalMat());
            nml::RenderState renderState(projMat, mvMat, cglib::pointwise_product(ambientLightColor, modelColor), cglib::pointwise_product(mainLightColor, modelColor), -mainLightDir);

            glModel->draw(*resourceManager, renderState);
        }

        // Remove stale models
        for (auto it = _nmlModelMap.begin(); it != _nmlModelMap.end(); ) {
            if (it->first.expired()) {
                it = _nmlModelMap.erase(it);
            } else {
                it++;
            }
        }

        // Dispose unused models
        resourceManager->deleteUnused();

        // Restore expected GL state
        glDepthMask(GL_FALSE);
        glActiveTexture(GL_TEXTURE0);

        GLContext::CheckGLError("NMLModelRenderer::onDrawFrame");
        return false;
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
    
    void NMLModelRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            const NMLModelDrawData& drawData = *element->getDrawData();
            
            std::shared_ptr<nml::Model> sourceModel = drawData.getSourceModel();
            auto modelIt = _nmlModelMap.find(sourceModel);
            if (modelIt == _nmlModelMap.end()) {
                continue;
            }
            std::shared_ptr<nml::GLModel> glModel = modelIt->second;
    
            cglib::mat4x4<double> modelMat = drawData.getLocalMat();
            cglib::mat4x4<double> invModelMat = cglib::inverse(modelMat);
    
            cglib::ray3<double> rayModel = cglib::transform_ray(ray, invModelMat);
            cglib::bbox3<double> modelBounds = cglib::bbox3<double>::convert(glModel->getBounds());
            if (!cglib::intersect_bbox(modelBounds, rayModel)) {
                continue;
            }

            std::vector<nml::RayIntersection> intersections;
            glModel->calculateRayIntersections(rayModel, intersections);
            
            for (std::size_t i = 0; i < intersections.size(); i++) {
                cglib::vec3<double> pos = cglib::transform_point(intersections[i].pos, modelMat);
                results.push_back(RayIntersectedElement(std::static_pointer_cast<VectorElement>(element), layer, pos, pos, true));
            }
        }
    }

    bool NMLModelRenderer::initializeRenderer() {
        if (_nmlResources && _nmlResources->isValid()) {
            return true;
        }

        if (auto mapRenderer = _mapRenderer.lock()) {
            _nmlResources = mapRenderer->getGLResourceManager()->create<NMLResources>();

            _nmlModelMap.clear();
        }

        return _nmlResources && _nmlResources->isValid();
    }

}
