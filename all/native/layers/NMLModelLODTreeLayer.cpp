#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "NMLModelLODTreeLayer.h"
#include "components/CancelableThreadPool.h"
#include "layers/NMLModelLODTreeEventListener.h"
#include "renderers/MapRenderer.h"
#include "renderers/NMLModelLODTreeRenderer.h"
#include "renderers/components/CullState.h"
#include "renderers/components/RayIntersectedElement.h"
#include "ui/NMLModelLODTreeClickInfo.h"
#include "utils/Log.h"

#include <nml/GLModel.h>
#include <nml/GLMesh.h>
#include <nml/GLTexture.h>
#include <nml/Package.h>

namespace {

    cglib::bbox3<double> calculateBounds(const carto::nml::Bounds3& bounds, const cglib::mat4x4<double>& matrix) {
        cglib::vec3<double> minBounds(bounds.min().x(), bounds.min().y(), bounds.min().z());
        cglib::vec3<double> maxBounds(bounds.max().x(), bounds.max().y(), bounds.max().z());
        return cglib::transform_bbox(cglib::bbox3<double>(minBounds, maxBounds), matrix);
    }
    
    float calculateProjectedScreenSize(const carto::nml::Bounds3& bounds, const cglib::mat4x4<double>& frustumMVP) {
        static const double NEAR_DISTANCE = 0.5;
    
        cglib::bbox3<double> projBounds = cglib::bbox3<double>::smallest();
        for (int i = 0; i < 8; i++) {
            cglib::vec4<double> point(0, 0, 0, 1);
            point(0) = ((i & 1) != 0 ? bounds.max().x() : bounds.min().x());
            point(1) = ((i & 2) != 0 ? bounds.max().y() : bounds.min().y());
            point(2) = ((i & 4) != 0 ? bounds.max().z() : bounds.min().z());
    
            cglib::vec4<double> projPoint = cglib::transform(point, frustumMVP);
            if (projPoint(3) < NEAR_DISTANCE) {
                projPoint(3) = NEAR_DISTANCE;
            }
    
            projBounds.add(cglib::vec3<double>(projPoint(0) / projPoint(3), projPoint(1) / projPoint(3), projPoint(2) / projPoint(3)));
        }
        cglib::vec3<double> projSize = projBounds.size();
        return (float) std::max(projSize(0), projSize(1));
    }
    
}
    
namespace carto {
    
    NMLModelLODTreeLayer::NMLModelLODTreeLayer(const std::shared_ptr<NMLModelLODTreeDataSource>& dataSource) :
        Layer(),
        _maxMemorySize(DEFAULT_MAX_MEMORY_SIZE),
        _LODResolutionFactor(1),
        _mapTileList(),
        _mapTileListViewState(),
        _modelLODTreeMap(),
        _modelLODTreeCache(DEFAULT_MODELLODTREE_CACHE_SIZE),
        _meshMap(),
        _meshCache(DEFAULT_MESH_CACHE_SIZE),
        _textureMap(),
        _textureCache(DEFAULT_TEXTURE_CACHE_SIZE),
        _nodeDrawDataMap(),
        _fetchingModelLODTrees(),
        _fetchingMeshes(),
        _fetchingTextures(),
        _fetchThreadPool(std::make_shared<CancelableThreadPool>()),
        _nmlModelLODTreeEventListener(),
        _dataSource(dataSource),
        _renderer(std::make_shared<NMLModelLODTreeRenderer>())
    {
        _fetchThreadPool->setPoolSize(1);
    }
    
    NMLModelLODTreeLayer::~NMLModelLODTreeLayer() {
        _fetchThreadPool->cancelAll();
        _fetchThreadPool->deinit();
    }
    
    std::shared_ptr<NMLModelLODTreeDataSource> NMLModelLODTreeLayer::getDataSource() const {
        return _dataSource;
    }

    unsigned int NMLModelLODTreeLayer::getMaxMemorySize() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _maxMemorySize;
    }    

    void NMLModelLODTreeLayer::setMaxMemorySize(unsigned int size) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _maxMemorySize = size;
        refresh();
    }

    float NMLModelLODTreeLayer::getLODResolutionFactor() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _LODResolutionFactor;
    }
    
    void NMLModelLODTreeLayer::setLODResolutionFactor(float factor) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _LODResolutionFactor = factor;
        refresh();
    }

    std::shared_ptr<NMLModelLODTreeEventListener> NMLModelLODTreeLayer::getNMLModelLODTreeEventListener() const {
        return _nmlModelLODTreeEventListener.get();
    }
    
    void NMLModelLODTreeLayer::setNMLModelLODTreeEventListener(const std::shared_ptr<NMLModelLODTreeEventListener>& nmlModelLODTreeEventListener) {
        _nmlModelLODTreeEventListener.set(nmlModelLODTreeEventListener);
    }
    
    bool NMLModelLODTreeLayer::isUpdateInProgress() const {
        return _fetchingModelLODTrees.getTaskCount() > 0 || _fetchingMeshes.getTaskCount() > 0 || _fetchingTextures.getTaskCount() > 0;
    }
    
    void NMLModelLODTreeLayer::offsetLayerHorizontally(double offset) {
        _renderer->offsetLayerHorizontally(offset);
    }
    
    void NMLModelLODTreeLayer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        Layer::onSurfaceCreated(shaderManager, textureManager);
        _renderer->onSurfaceCreated(shaderManager, textureManager);
    }
    
    bool NMLModelLODTreeLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState)
    {
        return _renderer->onDrawFrame(deltaSeconds, viewState);
    }
    
    void NMLModelLODTreeLayer::onSurfaceDestroyed(){
        _renderer->onSurfaceDestroyed();
        Layer::onSurfaceDestroyed();
    }
    
    void NMLModelLODTreeLayer::calculateRayIntersectedElements(const Projection& projection, const MapPos& rayOrig, const MapVec& rayDir, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::shared_ptr<NMLModelLODTreeLayer> thisLayer = std::static_pointer_cast<NMLModelLODTreeLayer>(std::const_pointer_cast<Layer>(shared_from_this()));
        _renderer->calculateRayIntersectedElements(thisLayer, rayOrig, rayDir, viewState, results);
    }

    bool NMLModelLODTreeLayer::processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const {
        std::shared_ptr<NMLModelLODTree::Proxy> element = intersectedElement.getElement<NMLModelLODTree::Proxy>();

        DirectorPtr<NMLModelLODTreeEventListener> nmlModelLODTreeEventListener = _nmlModelLODTreeEventListener;

        if (nmlModelLODTreeEventListener) {
            auto clickInfo = std::make_shared<NMLModelLODTreeClickInfo>(clickType, intersectedElement.getHitPos(), intersectedElement.getElementPos(), element->metaData, intersectedElement.getLayer());
            return nmlModelLODTreeEventListener->onNMLModelLODTreeClicked(clickInfo);
        }

        return clickType == ClickType::CLICK_TYPE_SINGLE || clickType == ClickType::CLICK_TYPE_LONG; // by default, disable 'click through' for single and long clicks
    }
    
    void NMLModelLODTreeLayer::registerDataSourceListener() {
    
    }
    
    void NMLModelLODTreeLayer::unregisterDataSourceListener() {
    
    }

    void NMLModelLODTreeLayer::setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                    const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                    const std::weak_ptr<Options>& options,
                                    const std::weak_ptr<MapRenderer>& mapRenderer,
                                    const std::weak_ptr<TouchHandler>& touchHandler)
    {
        Layer::setComponents(envelopeThreadPool, tileThreadPool, options, mapRenderer, touchHandler);
        _renderer->setOptions(options);
    }
    
    void NMLModelLODTreeLayer::loadData(const std::shared_ptr<CullState>& cullState) {
        float zoom = cullState->getViewState().getZoom();
        if (!isVisible() || !getVisibleZoomRange().inRange(zoom)) {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _renderer->refreshDrawData();
            return;
        }
    
        auto task = std::make_shared<MapTilesFetchTask>(std::static_pointer_cast<NMLModelLODTreeLayer>(shared_from_this()), cullState);
        if (_envelopeThreadPool) {
            _envelopeThreadPool->execute(task, getUpdatePriority());
        }
    }
    
    bool NMLModelLODTreeLayer::isDataAvailable(const NMLModelLODTree* modelLODTree, int nodeId) {
        return loadMeshes(modelLODTree, nodeId, true) && loadTextures(modelLODTree, nodeId, true);
    }	
    
    bool NMLModelLODTreeLayer::loadModelLODTrees(const MapTileList& mapTileList, bool checkOnly) {
        for (MapTileList::const_iterator it = mapTileList.begin(); it != mapTileList.end(); it++) {
            const NMLModelLODTreeDataSource::MapTile& mapTile = *it;
    
            ModelLODTreeMap::const_iterator modelLODTreeIt = _modelLODTreeMap.find(mapTile.modelLODTreeId);
            if (modelLODTreeIt == _modelLODTreeMap.end()) {
                std::shared_ptr<NMLModelLODTree> modelLODTree;
                if (_modelLODTreeCache.read(mapTile.modelLODTreeId, modelLODTree)) {
                    _modelLODTreeMap[mapTile.modelLODTreeId] = modelLODTree;
                } else {
                    if (checkOnly) {
                        return false;
                    }
                    if (!_fetchingModelLODTrees.exists(mapTile.modelLODTreeId)) {
                        auto task = std::make_shared<ModelLODTreeFetchTask>(std::static_pointer_cast<NMLModelLODTreeLayer>(shared_from_this()), mapTile);
                        _fetchThreadPool->execute(task, getUpdatePriority() + MODELLODTREE_LOADING_PRIORITY_OFFSET);
                    }
                }
            }
        }
        return true;
    }
    
    bool NMLModelLODTreeLayer::loadMeshes(const NMLModelLODTree* modelLODTree, int nodeId, bool checkOnly) {
        NMLModelLODTree::MeshBindingsMap::const_iterator mapIt = modelLODTree->getMeshBindingsMap().find(nodeId);
        if (mapIt == modelLODTree->getMeshBindingsMap().end()) {
            return false;
        }
    
        for (NMLModelLODTree::MeshBindingList::const_iterator listIt = mapIt->second.begin(); listIt != mapIt->second.end(); listIt++) {
            const NMLModelLODTree::MeshBinding& binding = *listIt;
    
            MeshMap::const_iterator meshIt = _meshMap.find(binding.meshId);
            if (meshIt == _meshMap.end()) {
                std::shared_ptr<nml::GLMesh> glMesh;
                if (_meshCache.read(binding.meshId, glMesh)) {
                    _meshMap[binding.meshId] = glMesh;
                } else {
                    if (checkOnly) {
                        return false;
                    }
                    if (!_fetchingMeshes.exists(binding.meshId)) {
                        auto task = std::make_shared<MeshFetchTask>(std::static_pointer_cast<NMLModelLODTreeLayer>(shared_from_this()), binding);
                        _fetchThreadPool->execute(task, getUpdatePriority() + MESH_LOADING_PRIORITY_OFFSET);
                    }
                }
            }
        }
        return true;
    }
    
    bool NMLModelLODTreeLayer::loadTextures(const NMLModelLODTree* modelLODTree, int nodeId, bool checkOnly) {
        NMLModelLODTree::TextureBindingsMap::const_iterator mapIt = modelLODTree->getTextureBindingsMap().find(nodeId);
        if (mapIt == modelLODTree->getTextureBindingsMap().end()) {
            return false;
        }
    
        for (NMLModelLODTree::TextureBindingList::const_iterator listIt = mapIt->second.begin(); listIt != mapIt->second.end(); listIt++) {
            const NMLModelLODTree::TextureBinding& binding = *listIt;
    
            TextureMap::const_iterator textureIt = _textureMap.find(binding.textureId);
            if (textureIt == _textureMap.end()) {
                std::shared_ptr<nml::GLTexture> glTexture;
                if (_textureCache.read(binding.textureId, glTexture)) {
                    _textureMap[binding.textureId] = glTexture;
                } else {
                    if (checkOnly) {
                        return false;
                    }
                    if (!_fetchingTextures.exists(binding.textureId)) {
                        auto task = std::make_shared<TextureFetchTask>(std::static_pointer_cast<NMLModelLODTreeLayer>(shared_from_this()), binding);
                        _fetchThreadPool->execute(task, getUpdatePriority() + TEXTURE_LOADING_PRIORITY_OFFSET);
                    }
                }
            }
        }
        return true;
    }
    
    void NMLModelLODTreeLayer::updateModelLODTrees(const MapTileList& mapTileList, ModelLODTreeMap& modelLODTreeMap) {
        for (MapTileList::const_iterator it = mapTileList.begin(); it != mapTileList.end(); it++) {
            const NMLModelLODTreeDataSource::MapTile& mapTile = *it;
            std::shared_ptr<NMLModelLODTree> modelLODTree;
            if (!_modelLODTreeCache.read(mapTile.modelLODTreeId, modelLODTree)) {
                ModelLODTreeMap::const_iterator modelLODTreeIt = _modelLODTreeMap.find(mapTile.modelLODTreeId);
                if (modelLODTreeIt == _modelLODTreeMap.end()) {
                    continue;
                }
                modelLODTree = modelLODTreeIt->second;
            }
            modelLODTreeMap[mapTile.modelLODTreeId] = modelLODTree;
        }
    }
    
    void NMLModelLODTreeLayer::updateMeshes(const NMLModelLODTree* modelLODTree, int nodeId, std::shared_ptr<nml::GLModel> glModel, MeshMap& meshMap) {
        NMLModelLODTree::MeshBindingsMap::const_iterator mapIt = modelLODTree->getMeshBindingsMap().find(nodeId);
        if (mapIt == modelLODTree->getMeshBindingsMap().end()) {
            return;
        }
    
        for (NMLModelLODTree::MeshBindingList::const_iterator listIt = mapIt->second.begin(); listIt != mapIt->second.end(); listIt++) {
            const NMLModelLODTree::MeshBinding& binding = *listIt;
    
            std::shared_ptr<nml::GLMesh> glMesh;
            if (!_meshCache.read(binding.meshId, glMesh)) {
                MeshMap::const_iterator meshIt = _meshMap.find(binding.meshId);
                if (meshIt == _meshMap.end()) {
                    continue;
                }
                glMesh = meshIt->second;
            }
            meshMap[binding.meshId] = glMesh;
    
            if (glModel) {
                glModel->replaceMesh(binding.localId, glMesh, binding.meshOp);
            }
        }
    }
    
    void NMLModelLODTreeLayer::updateTextures(const NMLModelLODTree* modelLODTree, int nodeId, std::shared_ptr<nml::GLModel> glModel, TextureMap& textureMap) {
        NMLModelLODTree::TextureBindingsMap::const_iterator mapIt = modelLODTree->getTextureBindingsMap().find(nodeId);
        if (mapIt == modelLODTree->getTextureBindingsMap().end()) {
            return;
        }
    
        for (NMLModelLODTree::TextureBindingList::const_iterator listIt = mapIt->second.begin(); listIt != mapIt->second.end(); listIt++) {
            const NMLModelLODTree::TextureBinding& binding = *listIt;
    
            std::shared_ptr<nml::GLTexture> glTexture;
            if (!_textureCache.read(binding.textureId, glTexture)) {
                TextureMap::const_iterator textureIt = _textureMap.find(binding.textureId);
                if (textureIt == _textureMap.end()) {
                    continue;
                }
                glTexture = textureIt->second;
            }
            textureMap[binding.textureId] = glTexture;
    
            if (glModel) {
                glModel->replaceTexture(binding.localId, glTexture);
            }
        }
    }
    
    void NMLModelLODTreeLayer::updateDrawLists(const ViewState& viewState, MeshMap& meshMap, TextureMap& textureMap, NodeDrawDataMap& nodeDrawDataMap) {
        typedef std::pair<const NMLModelLODTree *, int> Node;
        typedef std::pair<float, Node> SizeNodePair;
    
        cglib::mat4x4<double> mvpMatrix = viewState.getModelviewProjectionMat();
        cglib::frustum3<double> frustum = cglib::gl_projection_frustum(mvpMatrix);
    
        // Create initial node queue from roots
        std::priority_queue<SizeNodePair> initialQueue;
        for (ModelLODTreeMap::const_iterator it = _modelLODTreeMap.begin(); it != _modelLODTreeMap.end(); it++) {
            const NMLModelLODTree* modelLODTree = it->second.get();
            if (modelLODTree->getSourceNodeCount() == 0)
                continue;
    
            cglib::mat4x4<double> lmvpMatrix = mvpMatrix * modelLODTree->getLocalMat();
            float screenSize = calculateProjectedScreenSize(modelLODTree->getSourceNode(0)->bounds(), lmvpMatrix);
            initialQueue.push(SizeNodePair(screenSize, Node(modelLODTree, 0)));
        }
    
        // Create new queue by taking root nodes from initial queue until size limits are exceeded
        size_t totalSize = 0;
        std::priority_queue<SizeNodePair> queue;
        while (!initialQueue.empty()) {
            SizeNodePair sizeNodePair = initialQueue.top();
            initialQueue.pop();
    
            const NMLModelLODTree* modelLODTree = sizeNodePair.second.first;
            int nodeId = sizeNodePair.second.second;
            const nml::ModelLODTreeNode* node = modelLODTree->getSourceNode(nodeId);
    
            // If node is invisible, simply drop it and continue
            cglib::bbox3<double> modelBounds = calculateBounds(node->model().bounds(), modelLODTree->getLocalMat());
            if (!frustum.inside(modelBounds)) {
                continue;
            }
    
            // Test if this node can be added or we have already exceeded max memory footprint
            size_t nodeSize = node->model().texture_footprint() + node->model().mesh_footprint();
            if (totalSize + nodeSize <= _maxMemorySize) {
                queue.push(sizeNodePair);
                totalSize += nodeSize;
            }
        }
    
        // Create actual draw list by opening bigger nodes (as seen from viewpoint) first and maximum memory footprint is not exceeded
        std::vector<const nml::ModelLODTreeNode *> childList;
        std::vector<Node> nodeDrawList;
        while (!queue.empty()) {
            SizeNodePair sizeNodePair = queue.top();
            queue.pop();
    
            float screenSize = sizeNodePair.first;
            const NMLModelLODTree* modelLODTree = sizeNodePair.second.first;
            int nodeId = sizeNodePair.second.second;
            const nml::ModelLODTreeNode* node = modelLODTree->getSourceNode(nodeId);
    
            // Decide whether to subdivide this node - this depends on node screen size estimation and whether we can stay within memory size constraints after subdividing
            if (screenSize * _LODResolutionFactor > 2 && node->children_ids_size() > 0) {
                childList.clear();
                size_t nodeSize = node->model().texture_footprint() + node->model().mesh_footprint();
                size_t childListTotalSize = totalSize - nodeSize;
                for (int i = 0; i < node->children_ids_size(); i++) {
                    const nml::ModelLODTreeNode* childNode = modelLODTree->getSourceNode(node->children_ids(i));
    
                    // If child node is visible, add it to child list
                    cglib::bbox3<double> childModelBounds = calculateBounds(childNode->model().bounds(), modelLODTree->getLocalMat());
                    if (frustum.inside(childModelBounds)) {
                        childList.push_back(childNode);
                        size_t childNodeSize = childNode->model().texture_footprint() + childNode->model().mesh_footprint();
                        childListTotalSize += childNodeSize;
                    }
                }
                if (childListTotalSize <= _maxMemorySize) {
                    for (size_t i = 0; i < childList.size(); i++) {
                        const nml::ModelLODTreeNode* childNode = childList[i];
                        float screenSize = calculateProjectedScreenSize(childNode->bounds(), mvpMatrix * modelLODTree->getLocalMat());
                        queue.push(SizeNodePair(screenSize, Node(modelLODTree, childNode->id())));
                    }
                    totalSize = childListTotalSize;
                    continue;
                }
            }
    
            // Done with this node, add to draw list
            nodeDrawList.push_back(Node(modelLODTree, nodeId));
        }
    
        // Build children list for each parent node of current draw list
        std::map<long long, std::vector<int> > childrenIdsMap;
        for (size_t idx = 0; idx < nodeDrawList.size(); idx++) {
            const NMLModelLODTree* modelLODTree = nodeDrawList[idx].first;
            int nodeId = nodeDrawList[idx].second;
    
            for (int parentId = nodeId; parentId != 0; ) {
                parentId = modelLODTree->getNodeParentId(parentId);
                childrenIdsMap[modelLODTree->getGlobalNodeId(parentId)].push_back(nodeId);
            }
        }
    
        // Refine data - if data is not available for any nodes, try substitutions
        for (size_t idx = 0; idx < nodeDrawList.size(); ) {
            const NMLModelLODTree* modelLODTree = nodeDrawList[idx].first;
            int nodeId = nodeDrawList[idx].second;
    
            // If data is available, move to next node
            if (isDataAvailable(modelLODTree, nodeId)) {
                idx++;
                continue;
            }
    
            // Schedule data loading and remove this node from draw list
            loadMeshes(modelLODTree, nodeId, false);
            loadTextures(modelLODTree, nodeId, false);
            
            // Check if some children have data available. Size constraints are ignored from now on.
            int childNodeCounter = 0;
            std::vector<int> childIds = childrenIdsMap[modelLODTree->getGlobalNodeId(nodeId)];
            for (size_t j = 0; j < childIds.size(); j++) {
                int childId = childIds[j];
                if (isDataAvailable(modelLODTree, childId)) {
                    if (childNodeCounter++ == 0) {
                        nodeDrawList[idx] = Node(modelLODTree, childId);
                    } else {
                        nodeDrawList.insert(nodeDrawList.begin() + idx, Node(modelLODTree, childId));
                    }
                    idx++;
                    continue;
                }
                const std::vector<int>& childChildIds = childrenIdsMap[modelLODTree->getGlobalNodeId(childId)];
                childIds.insert(childIds.end(), childChildIds.begin(), childChildIds.end());
            }
            if (childNodeCounter > 0) {
                continue;
            }
    
            // Find closest parent that has data available. Ignore size constraints
            for (int parentId = nodeId; parentId != 0; ) {
                parentId = modelLODTree->getNodeParentId(parentId);
                if (isDataAvailable(modelLODTree, parentId)) {
                    nodeDrawList[idx] = Node(modelLODTree, parentId);
                    break;
                }
            }
            idx++;
        }
        
        // Create map of node ids
        std::map<long long, int> nodeIdsMap;
        for (size_t idx = 0; idx < nodeDrawList.size(); idx++) {
            const NMLModelLODTree* modelLODTree = nodeDrawList[idx].first;
            int nodeId = nodeDrawList[idx].second;
    
            nodeIdsMap[modelLODTree->getGlobalNodeId(nodeId)] = nodeId;
        }
    
        // Remove children of parent nodes already included
        for (size_t idx = 0; idx < nodeDrawList.size(); idx++) {
            const NMLModelLODTree* modelLODTree = nodeDrawList[idx].first;
            int nodeId = nodeDrawList[idx].second;
    
            for (int parentId = nodeId; parentId != 0; ) {
                parentId = modelLODTree->getNodeParentId(parentId);
                if (nodeIdsMap.find(modelLODTree->getGlobalNodeId(parentId)) != nodeIdsMap.end()) {
                    nodeDrawList.erase(nodeDrawList.begin() + idx);
                    idx--;
                    break;
                }
            }
        }
    
        // Load data
        for (size_t idx = 0; idx < nodeDrawList.size(); idx++) {
            const NMLModelLODTree* modelLODTree = nodeDrawList[idx].first;
            int nodeId = nodeDrawList[idx].second;
    
            std::shared_ptr<NMLModelLODTreeDrawData> nodeDrawData;
            NodeDrawDataMap::iterator it = _nodeDrawDataMap.find(modelLODTree->getGlobalNodeId(nodeId));
            if (it != _nodeDrawDataMap.end() && !it->second->isOffset()) {
                nodeDrawData = it->second;
            } else {
                auto glModel = std::make_shared<nml::GLModel>(modelLODTree->getSourceNode(nodeId)->model());
    
                std::vector<long long> globalParentIds;
                for (int parentId = nodeId; parentId != 0; ) {
                    parentId = modelLODTree->getNodeParentId(parentId);
                    globalParentIds.push_back(modelLODTree->getGlobalNodeId(parentId));
                }
    
                nodeDrawData = std::make_shared<NMLModelLODTreeDrawData>(std::static_pointer_cast<NMLModelLODTree>(const_cast<NMLModelLODTree *>(modelLODTree)->shared_from_this()), modelLODTree->getGlobalNodeId(nodeId), globalParentIds, glModel);
            }
    
            updateMeshes(modelLODTree, nodeId, nodeDrawData->getGLModel(), meshMap);
            updateTextures(modelLODTree, nodeId, nodeDrawData->getGLModel(), textureMap);
    
            nodeDrawDataMap[modelLODTree->getGlobalNodeId(nodeId)] = nodeDrawData;
            _renderer->addDrawData(nodeDrawData);
        }
    
        _renderer->refreshDrawData();
    
        if (std::shared_ptr<MapRenderer> mapRenderer = _mapRenderer.lock()) {
            mapRenderer->requestRedraw();
        }
    }
    
        NMLModelLODTreeLayer::MapTilesFetchTask::MapTilesFetchTask(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const std::shared_ptr<CullState>& cullState) :
        _layer(layer),
        _cullState(cullState)
    {
    }
    
    void NMLModelLODTreeLayer::MapTilesFetchTask::run() {
        const std::shared_ptr<NMLModelLODTreeLayer>& layer = _layer.lock();
        if (!layer) {
            return;
        }
    
        if (isCanceled()) {
            return;
        }
    
        std::unique_lock<std::recursive_mutex> lock(layer->_mutex);
        
        // If view has changed, cancel all previous task and fetch new list of map tiles
        if (layer->_mapTileListViewState.getModelviewProjectionMat() != _cullState->getViewState().getModelviewProjectionMat()) {
            layer->_fetchThreadPool->cancelAll();
            
            lock.unlock();
            MapTileList mapTileList = layer->_dataSource->loadMapTiles(_cullState);
            lock.lock();
    
            layer->_mapTileList = mapTileList;
            layer->_mapTileListViewState = _cullState->getViewState();
        }
    
        // Load new model LOD trees
        layer->loadModelLODTrees(layer->_mapTileList, false);
        ModelLODTreeMap modelLODTreeMap;
        layer->updateModelLODTrees(layer->_mapTileList, modelLODTreeMap);
        std::swap(layer->_modelLODTreeMap, modelLODTreeMap);
    
        // Create new draw lists, update renderer
        MeshMap meshMap;
        TextureMap textureMap;
        NodeDrawDataMap nodeDrawDataMap;
        layer->updateDrawLists(_cullState->getViewState(), meshMap, textureMap, nodeDrawDataMap);
        std::swap(layer->_meshMap, meshMap);
        std::swap(layer->_textureMap, textureMap);
        std::swap(layer->_nodeDrawDataMap, nodeDrawDataMap);
    }
    
    NMLModelLODTreeLayer::ModelLODTreeFetchTask::ModelLODTreeFetchTask(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const NMLModelLODTreeDataSource::MapTile& mapTile) :
        _layer(layer),
        _mapTile(mapTile)
    {
        layer->_fetchingModelLODTrees.add(_mapTile.modelLODTreeId);
    }
    
    void NMLModelLODTreeLayer::ModelLODTreeFetchTask::cancel() {
        CancelableTask::cancel();
    
        const std::shared_ptr<NMLModelLODTreeLayer>& layer = _layer.lock();
        if (!layer) {
            return;
        }
        layer->_fetchingModelLODTrees.remove(_mapTile.modelLODTreeId);
    }
    
    void NMLModelLODTreeLayer::ModelLODTreeFetchTask::run() {
        const std::shared_ptr<NMLModelLODTreeLayer>& layer = _layer.lock();
        if (!layer) {
            return;
        }
    
        if (isCanceled()) {
            return;
        }
    
        std::shared_ptr<NMLModelLODTree> modelLODTree = layer->_dataSource->loadModelLODTree(_mapTile);
        if (modelLODTree) {
            std::unique_lock<std::recursive_mutex> lock(layer->_mutex);
            layer->_modelLODTreeMap[_mapTile.modelLODTreeId] = modelLODTree;
            layer->_modelLODTreeCache.put(_mapTile.modelLODTreeId, modelLODTree, 1);
    
            if (std::shared_ptr<MapRenderer> mapRenderer = layer->_mapRenderer.lock()) {
                mapRenderer->layerChanged(layer->shared_from_this(), false);
            }
        }
        layer->_fetchingModelLODTrees.remove(_mapTile.modelLODTreeId);
    }
    
    NMLModelLODTreeLayer::MeshFetchTask::MeshFetchTask(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const NMLModelLODTree::MeshBinding& binding) :
        _layer(layer),
        _binding(binding)
    {
        layer->_fetchingMeshes.add(_binding.meshId);
    }
    
    void NMLModelLODTreeLayer::MeshFetchTask::cancel() {
        CancelableTask::cancel();
    
        const std::shared_ptr<NMLModelLODTreeLayer>& layer = _layer.lock();
        if (!layer) {
            return;
        }
        layer->_fetchingMeshes.remove(_binding.meshId);
    }
    
    void NMLModelLODTreeLayer::MeshFetchTask::run() {
        const std::shared_ptr<NMLModelLODTreeLayer>& layer = _layer.lock();
        if (!layer) {
            return;
        }
    
        if (isCanceled()) {
            return;
        }
    
        // Load new mesh
        std::shared_ptr<nml::Mesh> mesh = layer->_dataSource->loadMesh(_binding.meshId);
        if (mesh) {
            auto glMesh = std::make_shared<nml::GLMesh>(*mesh);
    
            std::unique_lock<std::recursive_mutex> lock(layer->_mutex);
            layer->_meshMap[_binding.meshId] = glMesh;
            layer->_meshCache.put(_binding.meshId, glMesh, glMesh->getTotalGeometrySize());
    
            if (std::shared_ptr<MapRenderer> mapRenderer = layer->_mapRenderer.lock()) {
                mapRenderer->layerChanged(layer->shared_from_this(), false);
            }
        }
        layer->_fetchingMeshes.remove(_binding.meshId);
    }
    
    NMLModelLODTreeLayer::TextureFetchTask::TextureFetchTask(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const NMLModelLODTree::TextureBinding& binding) :
        _layer(layer),
        _binding(binding)
    {
        layer->_fetchingTextures.add(_binding.textureId);
    }
    
    void NMLModelLODTreeLayer::TextureFetchTask::cancel() {
        CancelableTask::cancel();
    
        const std::shared_ptr<NMLModelLODTreeLayer>& layer = _layer.lock();
        if (!layer) {
            return;
        }
        layer->_fetchingTextures.remove(_binding.textureId);
    }
    
    void NMLModelLODTreeLayer::TextureFetchTask::run() {
        const std::shared_ptr<NMLModelLODTreeLayer>& layer = _layer.lock();
        if (!layer) {
            return;
        }
    
        if (isCanceled()) {
            return;
        }
    
        // Load new mesh
        std::shared_ptr<nml::Texture> texture = layer->_dataSource->loadTexture(_binding.textureId, _binding.level);
        if (texture) {
            auto glTexture = std::make_shared<nml::GLTexture>(texture);
    
            std::unique_lock<std::recursive_mutex> lock(layer->_mutex);
            layer->_textureMap[_binding.textureId] = glTexture;
            layer->_textureCache.put(_binding.textureId, glTexture, glTexture->getTextureSize());
    
            if (std::shared_ptr<MapRenderer> mapRenderer = layer->_mapRenderer.lock()) {
                mapRenderer->layerChanged(layer->shared_from_this(), false);
            }
        }
        layer->_fetchingTextures.remove(_binding.textureId);
    }
    
}

#endif
