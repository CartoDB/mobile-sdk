#include "layers/ClusteredVectorLayer.h"
#include "core/MapPos.h"
#include "core/MapTile.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "datasources/VectorDataSource.h"
#include "datasources/LocalVectorDataSource.h"
#include "layers/VectorLayer.h"
#include "layers/ClusterElementBuilder.h"
#include "renderers/MapRenderer.h"
#include "renderers/components/CullState.h"
#include "vectorelements/VectorElement.h"
#include "vectorelements/Point.h"
#include "vectorelements/Billboard.h"
#include "vectorelements/NMLModel.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <unordered_map>
#include <vector>
#include <list>
#include <stack>
#include <memory>
#include <utility>
#include <numeric>

#include <cglib/vec.h>

namespace carto {

    ClusteredVectorLayer::ClusteredVectorLayer(const std::shared_ptr<LocalVectorDataSource>& dataSource, const std::shared_ptr<ClusterElementBuilder>& clusterElementBuilder) :
        VectorLayer(dataSource),
        _clusterElementBuilder(clusterElementBuilder),
        _minClusterDistance(100),
        _maxClusterZoom(Const::MAX_SUPPORTED_ZOOM_LEVEL),
        _dpiScale(1),
        _rootCluster(),
        _refreshRootCluster(true),
        _renderClusters(),
        _clusterMutex()
    {
    }

    ClusteredVectorLayer::~ClusteredVectorLayer() {
    }

    std::shared_ptr<ClusterElementBuilder> ClusteredVectorLayer::getClusterElementBuilder() const {
        return _clusterElementBuilder.get();
    }

    float ClusteredVectorLayer::getMinimumClusterDistance() const {
        std::lock_guard<std::mutex> lock(_clusterMutex);
        return _minClusterDistance;
    }

    void ClusteredVectorLayer::setMinimumClusterDistance(float px) {
        {
            std::lock_guard<std::mutex> lock(_clusterMutex);
            _minClusterDistance = px;
        }

        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            mapRenderer = _mapRenderer.lock();
        }
        if (mapRenderer) {
            mapRenderer->requestRedraw();
        }
    }

    float ClusteredVectorLayer::getMaximumClusterZoom() const {
        std::lock_guard<std::mutex> lock(_clusterMutex);
        return _maxClusterZoom;
    }
    
    void ClusteredVectorLayer::setMaximumClusterZoom(float maxZoom) {
        {
            std::lock_guard<std::mutex> lock(_clusterMutex);
            _maxClusterZoom = maxZoom;
        }
        
        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            mapRenderer = _mapRenderer.lock();
        }
        if (mapRenderer) {
            mapRenderer->requestRedraw();
        }
    }
    
    bool ClusteredVectorLayer::expandCluster(const std::shared_ptr<VectorElement>& clusterElement, float px) {
        bool updated = false;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            std::stack<std::shared_ptr<Cluster> > clusters;
            clusters.push(_rootCluster);
            while (!clusters.empty()) {
                std::shared_ptr<Cluster> cluster = clusters.top();
                clusters.pop();
                if (!cluster) {
                    continue;
                }
                if (cluster->clusterElement == clusterElement) {
                    cluster->expandPx = px;
                    updated = true;
                    break;
                }
                clusters.push(cluster->subClusters[0]);
                clusters.push(cluster->subClusters[1]);
            }
        }
        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            mapRenderer = _mapRenderer.lock();
        }
        if (mapRenderer) {
            mapRenderer->requestRedraw();
        }
        return updated;
    }

    void ClusteredVectorLayer::refresh() {
        {
            std::lock_guard<std::mutex> lock(_clusterMutex);
            _refreshRootCluster = true;
        }
        VectorLayer::refresh();
    }

    bool ClusteredVectorLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState) {
        if (!isVisible() || !_lastCullState || !getVisibleZoomRange().inRange(viewState.getZoom())) {
            return false;
        }

        bool refresh = renderClusters(viewState, deltaSeconds);
        return VectorLayer::onDrawFrame(deltaSeconds, billboardSorter, styleCache, viewState) || refresh;
    }

    void ClusteredVectorLayer::refreshElement(const std::shared_ptr<VectorElement>& element, bool remove) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            if (_lastCullState) {
                syncRendererElement(element, _lastCullState->getViewState(), remove);
            }
        }
        refresh();
    }

    std::shared_ptr<CancelableTask> ClusteredVectorLayer::createFetchTask(const std::shared_ptr<CullState>& cullState) {
        return std::make_shared<ClusterFetchTask>(std::static_pointer_cast<ClusteredVectorLayer>(shared_from_this()));
    }

    ClusteredVectorLayer::ClusterFetchTask::ClusterFetchTask(const std::weak_ptr<ClusteredVectorLayer>& layer) :
        FetchTask(layer)
    {
    }

    bool ClusteredVectorLayer::ClusterFetchTask::loadElements(const std::shared_ptr<CullState> &cullState) {
        std::shared_ptr<ClusteredVectorLayer> layer = std::static_pointer_cast<ClusteredVectorLayer>(_layer.lock());

        if (auto options = layer->_options.lock()) {
            std::lock_guard<std::mutex> lock(layer->_clusterMutex);
            layer->_dpiScale = options->getDPI() / Const::UNSCALED_DPI;
        }

        std::vector<std::shared_ptr<VectorElement> > vectorElements = std::static_pointer_cast<LocalVectorDataSource>(layer->_dataSource.get())->getAll();
        bool refresh = false;
        {
            std::lock_guard<std::mutex> lock(layer->_clusterMutex);
            refresh = layer->_refreshRootCluster;
            if (!layer->_rootCluster) {
                refresh = true;
            }
            layer->_refreshRootCluster = false;
        }
        if (refresh) {
            std::shared_ptr<Cluster> rootCluster = layer->createClusters(vectorElements);

            std::lock_guard<std::mutex> lock(layer->_clusterMutex);
            layer->_rootCluster = rootCluster;
        }
        return false;
    }

    std::shared_ptr<ClusteredVectorLayer::Cluster> ClusteredVectorLayer::createClusters(const std::vector<std::shared_ptr<VectorElement> >& vectorElements) const {
        if (vectorElements.empty()) {
            return std::shared_ptr<Cluster>();
        }

        // Create singleton clusters
        std::vector<std::shared_ptr<Cluster> > clusters;
        clusters.reserve(vectorElements.size());
        for (const std::shared_ptr<VectorElement>& element : vectorElements) {
            std::shared_ptr<Cluster> cluster = createSingletonCluster(element);
            if (cluster) {
                clusters.push_back(cluster);
            }
        }

        // Merge into a single cluster
        return mergeClusters(clusters.begin(), clusters.end(), 1).front();
    }

    std::vector<std::shared_ptr<ClusteredVectorLayer::Cluster> > ClusteredVectorLayer::mergeClusters(std::vector<std::shared_ptr<Cluster> >::iterator clustersBegin, std::vector<std::shared_ptr<Cluster> >::iterator clustersEnd, size_t maxClusters) const {
        struct ClusterInfo {
            std::shared_ptr<Cluster> cluster;
            double closestClusterDistance;
            std::list<ClusterInfo>::iterator closestClusterInfoIt;
        };
        
        size_t initialClusters = clustersEnd - clustersBegin;

        // Find axis of bigger variance, use this axis for sorting
        cglib::vec2<double> mean = std::accumulate(clustersBegin, clustersEnd, cglib::vec2<double>::zero(), [initialClusters](const cglib::vec2<double>& result, const std::shared_ptr<Cluster>& cluster) {
            return result + cglib::vec2<double>(cluster->staticPos.getX() / initialClusters, cluster->staticPos.getY() / initialClusters);
        });
        cglib::vec2<double> variance = std::accumulate(clustersBegin, clustersEnd, cglib::vec2<double>::zero(), [mean](const cglib::vec2<double>& result, const std::shared_ptr<Cluster>& cluster) {
            cglib::vec2<double> delta = cglib::vec2<double>(cluster->staticPos.getX(), cluster->staticPos.getY()) - mean;
            return result + cglib::vec2<double>(delta(0) * delta(0), delta(1) * delta(1));
        });

        int axis = (variance(0) > variance(1) ? 0 : 1);
        auto clusterComparator = [axis](const std::shared_ptr<Cluster>& cluster1, const std::shared_ptr<Cluster>& cluster2) {
            return cluster1->staticPos[axis] < cluster2->staticPos[axis];
        };

        // Use hierarchical clustering, if size above threshold
        std::list<ClusterInfo> clusterInfos;
        if (initialClusters > 2 * THRESHOLD) {
            std::vector<std::shared_ptr<ClusteredVectorLayer::Cluster> > subClusters1;
            subClusters1.reserve(initialClusters);
            std::vector<std::shared_ptr<ClusteredVectorLayer::Cluster> > subClusters2;
            subClusters2.reserve(initialClusters);
            for (auto it = clustersBegin; it != clustersEnd; it++) {
                if ((*it)->staticPos[axis] < mean(axis)) {
                    subClusters1.push_back(*it);
                } else {
                    subClusters2.push_back(*it);
                }
            }
            if (!subClusters1.empty() && !subClusters2.empty()) {
                subClusters1 = mergeClusters(subClusters1.begin(), subClusters1.end(), THRESHOLD);
                subClusters2 = mergeClusters(subClusters2.begin(), subClusters2.end(), THRESHOLD);
                std::sort(subClusters1.begin(), subClusters1.end(), clusterComparator);
                std::sort(subClusters2.begin(), subClusters2.end(), clusterComparator);
                for (const std::shared_ptr<Cluster>& cluster : subClusters1) {
                    ClusterInfo clusterInfo;
                    clusterInfo.cluster = cluster;
                    clusterInfos.push_back(clusterInfo);
                }
                for (const std::shared_ptr<Cluster>& cluster : subClusters2) {
                    ClusterInfo clusterInfo;
                    clusterInfo.cluster = cluster;
                    clusterInfos.push_back(clusterInfo);
                }
            }
        }
        
        // If hierarchical clustering was not needed/did not succeed, use normal sorting/create full clustering
        if (clusterInfos.empty()) {
            std::sort(clustersBegin, clustersEnd, clusterComparator);
            for (auto it = clustersBegin; it != clustersEnd; it++) {
                ClusterInfo clusterInfo;
                clusterInfo.cluster = *it;
                clusterInfos.push_back(clusterInfo);
            }
        }

        // Find closest clusters. n*sqrt(n) steps avg, n^2 worst
        for (auto it = clusterInfos.begin(); it != clusterInfos.end(); it++) {
            it->closestClusterDistance = std::numeric_limits<double>::infinity();
            it->closestClusterInfoIt = clusterInfos.end();
            for (auto testIt = it; ++testIt != clusterInfos.end(); ) {
                if (testIt->cluster->staticPos[axis] - it->cluster->staticPos[axis] >= it->closestClusterDistance) {
                    break;
                }
                double distance = MapVec(testIt->cluster->staticPos - it->cluster->staticPos).length();
                if (distance < it->closestClusterDistance) {
                    it->closestClusterDistance = distance;
                    it->closestClusterInfoIt = testIt;
                }
            }
        }

        // Merge clusters one-by-one (n steps)
        while (clusterInfos.size() > maxClusters) {
            // Find closest pair. n steps
            std::list<ClusterInfo>::iterator it1 = clusterInfos.begin();
            for (auto it = clusterInfos.begin(); ++it != clusterInfos.end(); ) {
                if (it->closestClusterDistance < it1->closestClusterDistance) {
                    it1 = it;
                }
            }
            std::list<ClusterInfo>::iterator it2 = it1->closestClusterInfoIt;

            // Merge cluster pair
            ClusterInfo mergedClusterInfo;
            mergedClusterInfo.cluster = createMergedCluster(it1->cluster, it2->cluster);
            mergedClusterInfo.closestClusterDistance = std::numeric_limits<double>::infinity();
            mergedClusterInfo.closestClusterInfoIt = it2; // to force recalculation

            // Insert cluster to sorted cluster info list. 1 steps avg, n steps worst
            std::list<ClusterInfo>::iterator lastIt = clusterInfos.erase(it2);
            std::list<ClusterInfo>::iterator firstIt = clusterInfos.erase(it1);
            std::list<ClusterInfo>::iterator mergeIt;
            for (mergeIt = firstIt; mergeIt != lastIt; mergeIt++) {
                if (mergedClusterInfo.cluster->staticPos[axis] > mergeIt->cluster->staticPos[axis]) {
                    break;
                }
            }
            mergeIt = clusterInfos.insert(mergeIt, mergedClusterInfo);

            // For all clusters, if closest cluster is one of the merged clusters, recalculate closest cluster. n/2 steps avg
            for (auto it = clusterInfos.begin(); it != lastIt; it++) {
                if (it->closestClusterInfoIt == it1 || it->closestClusterInfoIt == it2) {
                    it->closestClusterDistance = std::numeric_limits<double>::infinity();
                    it->closestClusterInfoIt = clusterInfos.end();
                    for (auto testIt = it; ++testIt != clusterInfos.end(); ) {
                        if (testIt->cluster->staticPos[axis] - it->cluster->staticPos[axis] >= it->closestClusterDistance) {
                            break;
                        }
                        double distance = MapVec(testIt->cluster->staticPos - it->cluster->staticPos).length();
                        if (distance < it->closestClusterDistance) {
                            it->closestClusterDistance = distance;
                            it->closestClusterInfoIt = testIt;
                        }
                    }
                }
            }

            // Update clusters, check if merged cluster is closer than previously closest cluster. n/2 steps avg
            for (auto it = clusterInfos.begin(); it != mergeIt; it++) {
                double distance = MapVec(mergeIt->cluster->staticPos - it->cluster->staticPos).length();
                if (distance < it->closestClusterDistance) {
                    it->closestClusterInfoIt = mergeIt;
                    it->closestClusterDistance = distance;
                }
            }
        }

        // Return the remaining clusters
        std::vector<std::shared_ptr<Cluster> > clusters;
        clusters.reserve(maxClusters);
        std::for_each(clusterInfos.begin(), clusterInfos.end(), [&clusters](const ClusterInfo& clusterInfo) { clusters.push_back(clusterInfo.cluster); });
        return clusters;
    }

    std::shared_ptr<ClusteredVectorLayer::Cluster> ClusteredVectorLayer::createSingletonCluster(const std::shared_ptr<VectorElement>& element) const {
        std::shared_ptr<Cluster> cluster;
        MapPos mapPos;
        if (GetVectorElementPos(element, mapPos)) {
            cluster = std::make_shared<Cluster>();
            cluster->maxDistance = 0;
            cluster->expandPx = 0;
            cluster->staticPos = cluster->transitionPos = mapPos;
            cluster->mapBoundsInternal = MapBounds(_dataSource->getProjection()->toInternal(mapPos), _dataSource->getProjection()->toInternal(mapPos));
            cluster->elements.insert(element);

            cluster->clusterElement = _clusterElementBuilder->buildClusterElement(cluster->transitionPos, std::vector<std::shared_ptr<VectorElement> >(cluster->elements.begin(), cluster->elements.end()));
        }
        return cluster;
    }

    std::shared_ptr<ClusteredVectorLayer::Cluster> ClusteredVectorLayer::createMergedCluster(const std::shared_ptr<Cluster>& cluster1, const std::shared_ptr<Cluster>& cluster2) const {
        int n1 = static_cast<int>(cluster1->elements.size());
        int n2 = static_cast<int>(cluster2->elements.size());
        MapPos mapPos((cluster1->staticPos.getX() * n1 + cluster2->staticPos.getX() * n2) / (n1 + n2), (cluster1->staticPos.getY() * n1 + cluster2->staticPos.getY() * n2) / (n1 + n2));
        double dist = MapVec(_dataSource->getProjection()->toInternal(cluster1->staticPos) - _dataSource->getProjection()->toInternal(cluster2->staticPos)).length();

        auto cluster = std::make_shared<Cluster>();
        cluster->maxDistance = dist;
        cluster->expandPx = 0;
        cluster->staticPos = cluster->transitionPos = mapPos;
        cluster->mapBoundsInternal.expandToContain(cluster1->mapBoundsInternal);
        cluster->mapBoundsInternal.expandToContain(cluster2->mapBoundsInternal);
        cluster->elements.insert(cluster1->elements.begin(), cluster1->elements.end());
        cluster->elements.insert(cluster2->elements.begin(), cluster2->elements.end());
        cluster->subClusters[0] = cluster1;
        cluster->subClusters[1] = cluster2;
        cluster1->parentCluster = cluster;
        cluster2->parentCluster = cluster;

        cluster->clusterElement = _clusterElementBuilder->buildClusterElement(cluster->transitionPos, std::vector<std::shared_ptr<VectorElement> >(cluster->elements.begin(), cluster->elements.end()));
        return cluster;
    }

    bool ClusteredVectorLayer::renderClusters(const ViewState& viewState, float deltaSeconds) {
        std::lock_guard<std::mutex> lock(_clusterMutex);

        // Initialize render state, use previously renderered cluster list
        RenderState renderState;
        MapPos p0 = viewState.screenToWorldPlane(ScreenPos(viewState.getHalfWidth() + 0, viewState.getHalfHeight()), 0);
        MapPos p1 = viewState.screenToWorldPlane(ScreenPos(viewState.getHalfWidth() + 1, viewState.getHalfHeight()), 0);
        renderState.pixelMeasure = MapVec(p1 - p0).length() * _dpiScale;
        renderState.totalExpanded = 0;
        renderState.visibleClusterSet.insert(_renderClusters.begin(), _renderClusters.end());
        for (const std::shared_ptr<Cluster>& cluster : _renderClusters) {
            for (std::shared_ptr<Cluster> parentCluster = cluster->parentCluster.lock(); parentCluster; parentCluster = parentCluster->parentCluster.lock()) {
                renderState.visibleChildClusterMap[parentCluster].push_back(cluster);
            }
        }

        // Create new rendering list
        _renderClusters.clear();
        bool refresh = renderCluster(_rootCluster, viewState, renderState, deltaSeconds);
        
        // First pass, create rendering elements from scratch
        for (const std::shared_ptr<Cluster>& cluster : _renderClusters) {
            std::shared_ptr<VectorElement> element = cluster->clusterElement;
            if ((cluster->elements.size() == 1 && cluster->transitionPos == cluster->staticPos)) {
                element = *cluster->elements.begin();
            }
            addRendererElement(element);
        }

        bool billboardsChanged = refreshRendererElements();

        // Second pass, update positions of clustering elements
        for (const std::shared_ptr<Cluster>& cluster : _renderClusters) {
            if (!(cluster->elements.size() == 1 && cluster->transitionPos == cluster->staticPos)) {
                SetVectorElementPos(cluster->clusterElement, cluster->transitionPos);
                billboardsChanged = syncRendererElement(cluster->clusterElement, viewState, false) || billboardsChanged;
            }
        }

        // If billboards were changed and no animations running, notify map renderer
        if (billboardsChanged && !refresh) {
            std::shared_ptr<MapRenderer> mapRenderer = _mapRenderer.lock();
            if (mapRenderer) {
                mapRenderer->billboardsChanged();
            }
        }
        
        return refresh;
    }

    bool ClusteredVectorLayer::renderCluster(const std::shared_ptr<Cluster>& cluster, const ViewState& viewState, RenderState& renderState, float deltaSeconds) {
        if (!cluster) {
            return false;
        }
        if (!viewState.getFrustum().squareIntersects(cluster->mapBoundsInternal)) {
            return false;
        }

        // Handle expanded clusters. Expanded clusters are always fully expanded
        bool stop = false;
        if (viewState.getZoom() < _maxClusterZoom) {
            if (!renderState.expandedCluster && cluster->expandPx > 0) {
                moveCluster(cluster, cluster->staticPos, renderState, deltaSeconds);
                renderState.expandedCluster = cluster;
                renderState.totalExpanded = 0;
            } else {
                stop = cluster->maxDistance < _minClusterDistance * renderState.pixelMeasure;
            }
        }
        if (cluster->elements.size() == 1 || stop) {
            return animateCluster(cluster, renderState, deltaSeconds);
        }

        // Draw subclusters recursively
        bool refresh = false;
        if (renderCluster(cluster->subClusters[0], viewState, renderState, deltaSeconds)) {
            refresh = true;
        }
        if (renderCluster(cluster->subClusters[1], viewState, renderState, deltaSeconds)) {
            refresh = true;
        }

        // Undo expanded state
        if (renderState.expandedCluster == cluster) {
            renderState.expandedCluster.reset();
        }
        return refresh;
    }

    bool ClusteredVectorLayer::animateCluster(const std::shared_ptr<Cluster>& cluster, RenderState& renderState, float deltaSeconds) {
        bool refresh = false;

        // Transition all visible children from actual position to parent position
        auto it = renderState.visibleChildClusterMap.find(cluster);
        if (it != renderState.visibleChildClusterMap.end()) {
            for (const std::shared_ptr<Cluster>& childCluster : it->second) {
                if (moveCluster(childCluster, cluster->staticPos, renderState, deltaSeconds)) {
                    refresh = true;
                    _renderClusters.push_back(childCluster);
                }
            }
        }
        if (refresh) {
            return refresh;
        }

        // Transition from expanded parent to actual expanded positions
        if (renderState.expandedCluster) {
            if (renderState.visibleClusterSet.find(renderState.expandedCluster) != renderState.visibleClusterSet.end()) {
                cluster->transitionPos = renderState.expandedCluster->transitionPos;
            }
            if (moveCluster(cluster, createExpandedElementPos(renderState), renderState, deltaSeconds)) {
                refresh = true;
            }
            _renderClusters.push_back(cluster);
            return refresh;
        }

        // Transition from visible parent to actual position
        for (std::shared_ptr<Cluster> parentCluster = cluster; parentCluster; parentCluster = parentCluster->parentCluster.lock()) {
            if (renderState.visibleClusterSet.find(parentCluster) != renderState.visibleClusterSet.end()) {
                cluster->transitionPos = parentCluster->transitionPos;
                if (moveCluster(cluster, cluster->staticPos, renderState, deltaSeconds)) {
                    refresh = true;
                }
                _renderClusters.push_back(cluster);
                return refresh;
            }
        }

        // New cluster
        cluster->transitionPos = cluster->staticPos;
        _renderClusters.push_back(cluster);
        return refresh;
    }

    bool ClusteredVectorLayer::moveCluster(const std::shared_ptr<Cluster>& cluster, const MapPos& targetPos, const RenderState& renderState, float deltaSeconds) {
        MapVec deltaPos(_dataSource->getProjection()->toInternal(targetPos) - _dataSource->getProjection()->toInternal(cluster->transitionPos));
        if (deltaPos.length() <= renderState.pixelMeasure * 0.25) {
            cluster->transitionPos = targetPos;
            return false;
        }
        for (float t = 0; t < 0.05f && t < deltaSeconds; t += 0.0015f) {
            cluster->transitionPos = MapPos(cluster->transitionPos.getX() * 0.99 + targetPos.getX() * 0.01, cluster->transitionPos.getY() * 0.99 + targetPos.getY() * 0.01);
        }
        return true;
    }

    MapPos ClusteredVectorLayer::createExpandedElementPos(RenderState &renderState) const {
        MapPos mapPos = _dataSource->getProjection()->toInternal(renderState.expandedCluster->transitionPos);
        double angle = Const::PI * 2 * renderState.totalExpanded++ / renderState.expandedCluster->elements.size();
        double dist = renderState.expandedCluster->expandPx * renderState.pixelMeasure;
        return _dataSource->getProjection()->fromInternal(mapPos + MapVec(std::cos(angle), std::sin(angle)) * dist);
    }

    bool ClusteredVectorLayer::GetVectorElementPos(const std::shared_ptr<VectorElement>& vectorElement, MapPos& pos) {
        std::shared_ptr<Geometry> geometry = vectorElement->getGeometry();
        if (auto pointGeometry = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            pos = pointGeometry->getPos();
            return true;
        }
        return false;
    }

    bool ClusteredVectorLayer::SetVectorElementPos(const std::shared_ptr<VectorElement>& vectorElement, const MapPos& pos) {
        if (auto point = std::dynamic_pointer_cast<Point>(vectorElement)) {
            point->setPos(pos);
            return true;
        } else if (auto billboard = std::dynamic_pointer_cast<Billboard>(vectorElement)) {
            billboard->setPos(pos);
            return true;
        } else if (auto nmlModel = std::dynamic_pointer_cast<NMLModel>(vectorElement)) {
            nmlModel->setPos(pos);
            return true;
        }
        return false;
    }

}
