#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "NMLModelLODTreeRenderer.h"
#include "components/ThreadWorker.h"
#include "datasources/NMLModelLODTreeDataSource.h"
#include "graphics/ViewState.h"
#include "layers/NMLModelLODTreeLayer.h"
#include "projections/ProjectionSurface.h"
#include "renderers/MapRenderer.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/NMLResources.h"
#include "utils/Log.h"

#include <nml/GLModel.h>
#include <nml/GLTexture.h>
#include <nml/GLResourceManager.h>

namespace carto {

    NMLModelLODTreeRenderer::NMLModelLODTreeRenderer() :
        _mapRenderer(),
        _options(),
        _nmlResources(),
        _tempDrawDatas(),
        _drawRecordMap(),
        _mutex()
    {
    }
    
    NMLModelLODTreeRenderer::~NMLModelLODTreeRenderer() {
    }
    
    void NMLModelLODTreeRenderer::setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer) {
        std::lock_guard<std::mutex> lock(_mutex);

        _options = options;
        _mapRenderer = mapRenderer;
        _nmlResources.reset();
    }

    void NMLModelLODTreeRenderer::offsetLayerHorizontally(double offset) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (auto it = _drawRecordMap.begin(); it != _drawRecordMap.end(); it++) {
            ModelNodeDrawRecord& record = *it->second;
            record.drawData.offsetHorizontally(offset);
        }
    }

    bool NMLModelLODTreeRenderer::onDrawFrame(float deltaSeconds, const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);
    
        std::shared_ptr<Options> options = _options.lock();
        if (!options) {
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

        // Create new models
        cglib::mat4x4<float> projMat = cglib::mat4x4<float>::convert(viewState.getProjectionMat());
        for (auto it = _drawRecordMap.begin(); it != _drawRecordMap.end(); it++) {
            ModelNodeDrawRecord& record = *it->second;
            if (!(record.used && !record.created)) {
                continue;
            }
    
            record.drawData.getGLModel()->create(*resourceManager);

            record.created = true;
        }
    
        // If a model is not used but created then keep it if it has a parent that is used but not created. Also check that it does not have a closer parent that is created.  
        for (auto it = _drawRecordMap.begin(); it != _drawRecordMap.end(); it++) {
            ModelNodeDrawRecord& record = *it->second;
            if (!(!record.used && record.created)) {
                continue;
            }
    
            for (ModelNodeDrawRecord* parentRecord = record.parent; parentRecord; parentRecord = parentRecord->parent) {
                if (parentRecord->created) {
                    break;
                }
                if (parentRecord->used) {
                    record.used = true;
                    break;
                }
            }
        }
    
        // Draw nodes if they do not have parents that are also used/created
        for (auto it = _drawRecordMap.begin(); it != _drawRecordMap.end(); it++) {
            ModelNodeDrawRecord& record = *it->second;
            if (!record.used || !record.created) {
                continue;
            }
    
            bool draw = true;
            for (ModelNodeDrawRecord* parentRecord = record.parent; parentRecord; parentRecord = parentRecord->parent) {
                if (parentRecord->used && parentRecord->created) {
                    draw = false;
                    break;
                }
            }
            if (!draw) {
                continue;
            }

            cglib::mat4x4<float> mvMat = cglib::mat4x4<float>::convert(viewState.getModelviewMat() * record.drawData.getLocalMat());
            nml::RenderState renderState(projMat, mvMat, ambientLightColor, mainLightColor, mainLightDir);

            record.drawData.getGLModel()->draw(*resourceManager, renderState);
        }
    
        // Remove all unused models, update parent-child links
        for (auto it = _drawRecordMap.begin(); it != _drawRecordMap.end(); ) {
            ModelNodeDrawRecord& record = *it->second;
            if (record.used) {
                it++;
                continue;
            }
    
            if (record.parent) {
                auto childIt = std::find(record.parent->children.begin(), record.parent->children.end(), &record);
                record.parent->children.erase(childIt);
                record.parent->children.insert(record.parent->children.end(), record.children.begin(), record.children.end());
            }
            for (std::size_t i = 0; i < record.children.size(); i++) {
                record.children[i]->parent = record.parent;
            }
    
            _drawRecordMap.erase(it++);
        }
    
        // Release unused resources
        resourceManager->deleteUnused();
        
        // Restore expected GL state
        glDepthMask(GL_FALSE);
        glActiveTexture(GL_TEXTURE0);

        return false;
    }

    void NMLModelLODTreeRenderer::addDrawData(const std::shared_ptr<NMLModelLODTreeDrawData>& drawData) {
        _tempDrawDatas.push_back(drawData);
    }
    
    void NMLModelLODTreeRenderer::refreshDrawData() {
        std::lock_guard<std::mutex> lock(_mutex);
    
        // Mark all existing records as unused, unlink node hierarchy
        for (auto it = _drawRecordMap.begin(); it != _drawRecordMap.end(); it++) {
            ModelNodeDrawRecord& record = *it->second;
            record.used = false;
            record.parent = nullptr;
            record.children.clear();
        }
    
        // Create new nodes, mark used nodes
        for (auto it = _tempDrawDatas.begin(); it != _tempDrawDatas.end(); it++) {
            std::shared_ptr<ModelNodeDrawRecord>& record = _drawRecordMap[(*it)->getNodeId()];
            if (!record) {
                record.reset(new ModelNodeDrawRecord(**it));
            } else {
                record->drawData = **it;
            }
            record->used = true;
        }
    
        // Build node hierarchy, create parent-child links between draw records
        for (auto it = _drawRecordMap.begin(); it != _drawRecordMap.end(); it++) {
            ModelNodeDrawRecord& record = *it->second;
            for (std::size_t i = 0; i < record.drawData.getParentIds().size(); i++) {
                auto it2 = _drawRecordMap.find(record.drawData.getParentIds()[i]);
                if (it2 != _drawRecordMap.end()) {
                    ModelNodeDrawRecord& parentRecord = *it2->second;
                    record.parent = &parentRecord;
                    parentRecord.children.push_back(&record);
                    break;
                }
            }
        }
    
        _tempDrawDatas.clear();
    }
    
    void NMLModelLODTreeRenderer::calculateRayIntersectedElements(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::lock_guard<std::mutex> lock(_mutex);
    
        for (auto it = _drawRecordMap.begin(); it != _drawRecordMap.end(); it++) {
            const ModelNodeDrawRecord& record = *it->second;
            if (!(record.used && record.created)) {
                continue;
            }
    
            std::shared_ptr<nml::GLModel> glModel = record.drawData.getGLModel();
            const cglib::mat4x4<double>& modelMat = record.drawData.getLocalMat();
            cglib::mat4x4<double> invModelMat = cglib::inverse(modelMat);
    
            cglib::ray3<double> rayModel = cglib::transform_ray(ray, invModelMat);
            cglib::bbox3<double> modelBounds = cglib::bbox3<double>::convert(glModel->getBounds());
            if (!cglib::intersect_bbox(modelBounds, rayModel)) {
                continue;
            }
            
            std::vector<nml::RayIntersection> intersections;
            glModel->calculateRayIntersections(rayModel, intersections);
            
            for (std::size_t i = 0; i < intersections.size(); i++) {
                auto proxyIt = record.drawData.getProxyMap()->find(intersections[i].vertexId);
                if (proxyIt == record.drawData.getProxyMap()->end()) {
                    continue;
                }
                
                cglib::vec3<double> pos = cglib::transform_point(intersections[i].pos, modelMat);
                results.push_back(RayIntersectedElement(std::make_shared<NMLModelLODTree::Proxy>(proxyIt->second), layer, pos, pos, true));
            }
        }
    }

    bool NMLModelLODTreeRenderer::initializeRenderer() {
        if (_nmlResources && _nmlResources->isValid()) {
            return true;
        }

        if (auto mapRenderer = _mapRenderer.lock()) {
            _nmlResources = mapRenderer->getGLResourceManager()->create<NMLResources>();
        }

        return _nmlResources && _nmlResources->isValid();
    }

}

#endif
