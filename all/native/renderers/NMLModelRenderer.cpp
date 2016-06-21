#include "NMLModelRenderer.h"
#include "datasources/VectorDataSource.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "projections/Projection.h"
#include "renderers/components/RayIntersectedElement.h"
#include "utils/Log.h"
#include "utils/GLES2.h"
#include "utils/GeomUtils.h"
#include "utils/GLUtils.h"

#include <nml/GLModel.h>
#include <nml/GLShaderManager.h>

namespace carto {

    NMLModelRenderer::NMLModelRenderer() :
        _glShaderManager(),
        _glModelMap(),
        _elements(),
        _tempElements(),
        _options(),
        _mutex()
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
    
    void NMLModelRenderer::setOptions(const std::weak_ptr<Options>& options) {
        std::lock_guard<std::mutex> lock(_mutex);
        _options = options;
    }

    void NMLModelRenderer::offsetLayerHorizontally(double offset) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            element->getDrawData()->offsetHorizontally(offset);
        }
    }

    void NMLModelRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _glShaderManager = std::make_shared<nml::GLShaderManager>();
        _glModelMap.clear();
    }

    bool NMLModelRenderer::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);

        std::shared_ptr<Options> options = _options.lock();
        if (!options) {
            return false;
        }
    
        // Set expected GL state
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        // Calculate lighting state
        Color ambientColor = options->getAmbientLightColor();
        cglib::vec4<float> ambientLightColor = cglib::vec4<float>(ambientColor.getR(), ambientColor.getG(), ambientColor.getB(), ambientColor.getA()) * (1.0f / 255.0f);
        Color mainColor = options->getMainLightColor();
        cglib::vec4<float> mainLightColor = cglib::vec4<float>(mainColor.getR(), mainColor.getG(), mainColor.getB(), mainColor.getA()) * (1.0f / 255.0f);
        MapVec mainDir = options->getMainLightDirection();
        cglib::vec3<float> mainLightDir = cglib::unit(cglib::vec3<float>::convert(cglib::transform_vector(cglib::vec3<double>(mainDir.getX(), mainDir.getY(), mainDir.getZ()), viewState.getModelviewMat())));

        // Draw models
        cglib::mat4x4<float> projMat = cglib::mat4x4<float>::convert(viewState.getProjectionMat());
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            const NMLModelDrawData& drawData = *element->getDrawData();
            std::shared_ptr<nml::Model> sourceModel = drawData.getSourceModel();
            std::shared_ptr<nml::GLModel> glModel = _glModelMap[sourceModel];
            if (!glModel) {
                glModel = std::make_shared<nml::GLModel>(*sourceModel);
                glModel->create(*_glShaderManager);
                _glModelMap[sourceModel] = glModel;
            }
    
            cglib::mat4x4<float> mvMat = cglib::mat4x4<float>::convert(viewState.getModelviewMat() * drawData.getLocalMat());
            nml::RenderState renderState(projMat, mvMat, ambientLightColor, mainLightColor, -mainLightDir);

            glModel->draw(renderState);
        }
    
        // Dispose unused models
        for (auto it = _glModelMap.begin(); it != _glModelMap.end(); ) {
            if (it->first.unique()) {
                it->second->dispose();
                it = _glModelMap.erase(it);
            } else {
                it++;
            }
        }

        // Restore expected GL state
        glDepthMask(GL_TRUE);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);

        GLUtils::checkGLError("NMLModelRenderer::onDrawFrame()");
        return false;
    }

    void NMLModelRenderer::onSurfaceDestroyed() {
        _glShaderManager.reset();
        _glModelMap.clear();
    }

    void NMLModelRenderer::calculateRayIntersectedElements(const std::shared_ptr<VectorLayer>& layer, const MapPos& rayOrig, const MapVec& rayDir, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (const std::shared_ptr<NMLModel>& element : _elements) {
            const NMLModelDrawData& drawData = *element->getDrawData();
            
            std::shared_ptr<nml::Model> sourceModel = drawData.getSourceModel();
            auto modelIt = _glModelMap.find(sourceModel);
            if (modelIt == _glModelMap.end()) {
                continue;
            }
            std::shared_ptr<nml::GLModel> glModel = modelIt->second;
    
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
            
            std::vector<nml::RayIntersection> intersections;
            glModel->calculateRayIntersections(nml::Ray(rayOrigModel, rayDirModel), intersections);
            
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
    
}
