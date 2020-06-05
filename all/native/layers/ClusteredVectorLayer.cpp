#include "layers/ClusteredVectorLayer.h"
#include "core/MapPos.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "datasources/VectorDataSource.h"
#include "datasources/LocalVectorDataSource.h"
#include "layers/VectorLayer.h"
#include "layers/ClusterElementBuilder.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
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
        _clusterBuilderMode(),
        _minClusterDistance(100),
        _maxClusterZoom(Const::MAX_SUPPORTED_ZOOM_LEVEL),
        _animatedClusters(true),
        _dpiScale(1),
        _clusters(std::make_shared<std::vector<Cluster> >()),
        _projectionSurface(),
        _singletonClusterCount(0),
        _rootClusterIdx(-1),
        _renderClusterIdxs(),
        _refreshRootCluster(true),
        _clusterMutex()
    {
        if (!clusterElementBuilder) {
            throw NullArgumentException("Null clusterElementBuilder");
        }

        _clusterBuilderMode = clusterElementBuilder->getBuilderMode();
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
        redraw();
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
        redraw();
    }

    bool ClusteredVectorLayer::isAnimatedClusters() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _animatedClusters;
    }

    void ClusteredVectorLayer::setAnimatedClusters(bool animated) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _animatedClusters = animated; // NOTE: no need to refresh
    }
    
    bool ClusteredVectorLayer::expandCluster(const std::shared_ptr<VectorElement>& clusterElement, float px) {
        bool updated = false;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            std::stack<int> clusterIdxs;
            clusterIdxs.push(_rootClusterIdx);
            while (!clusterIdxs.empty()) {
                int clusterIdx = clusterIdxs.top();
                clusterIdxs.pop();
                if (clusterIdx == -1) {
                    continue;
                }
                Cluster& cluster = (*_clusters)[clusterIdx];
                if (cluster.clusterElement == clusterElement) {
                    cluster.expandPx = px;
                    updated = true;
                    break;
                }
                clusterIdxs.push(cluster.childClusterIdx[0]);
                clusterIdxs.push(cluster.childClusterIdx[1]);
            }
        }
        redraw();
        return updated;
    }

    void ClusteredVectorLayer::refresh() {
        {
            std::lock_guard<std::mutex> lock(_clusterMutex);
            _refreshRootCluster = true;
        }
        VectorLayer::refresh();
    }

    bool ClusteredVectorLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState) {
        if (!isVisible() || !_lastCullState || !getVisibleZoomRange().inRange(viewState.getZoom()) || getOpacity() <= 0) {
            return false;
        }

        bool refresh = renderClusters(viewState, deltaSeconds);
        return VectorLayer::onDrawFrame(deltaSeconds, billboardSorter, viewState) || refresh;
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

    bool ClusteredVectorLayer::ClusterFetchTask::loadElements(const std::shared_ptr<CullState>& cullState) {
        std::shared_ptr<ClusteredVectorLayer> layer = std::static_pointer_cast<ClusteredVectorLayer>(_layer.lock());
        if (!layer) {
            return false;
        }

        if (auto options = layer->getOptions()) {
            std::lock_guard<std::mutex> lock(layer->_clusterMutex);
            layer->_dpiScale = options->getDPI() / Const::UNSCALED_DPI;
        }

        std::vector<std::shared_ptr<VectorElement> > vectorElements = std::static_pointer_cast<LocalVectorDataSource>(layer->_dataSource.get())->getAll();
        bool refresh = false;
        {
            std::lock_guard<std::mutex> lock(layer->_clusterMutex);
            std::swap(refresh, layer->_refreshRootCluster);
        }
        if (refresh) {
            layer->rebuildClusters(vectorElements);
        }
        return false;
    }

    void ClusteredVectorLayer::rebuildClusters(const std::vector<std::shared_ptr<VectorElement> >& vectorElements) {
        std::shared_ptr<ProjectionSurface> projectionSurface;
        if (auto mapRenderer = getMapRenderer()) {
            projectionSurface = mapRenderer->getProjectionSurface();
        }
        if (!projectionSurface) {
            return;
        }

        auto clusters = std::make_shared<std::vector<Cluster> >();
        clusters->reserve(vectorElements.size() * 2);
        int singletonClusterCount = 0;
        int rootClusterIdx = -1;
        if (!vectorElements.empty()) {
            // Create singleton clusters
            std::vector<int> clusterIdxs;
            clusterIdxs.reserve(vectorElements.size());
            for (const std::shared_ptr<VectorElement>& element : vectorElements) {
                int clusterIdx = createSingletonCluster(element, *clusters, *projectionSurface);
                if (clusterIdx != -1) {
                    clusterIdxs.push_back(clusterIdx);
                }
            }
            singletonClusterCount = static_cast<int>(clusters->size());

            // Check if we must recalculate clustering
            {
                std::lock_guard<std::mutex> lock(_clusterMutex);

                if (_singletonClusterCount == singletonClusterCount) {
                    bool changed = false;
                    for (int i = 0; i < singletonClusterCount; i++) {
                        if ((*_clusters)[i].vectorElement != (*clusters)[i].vectorElement ||
                            (*_clusters)[i].staticPos != (*clusters)[i].staticPos)
                        {
                            changed = true;
                            break;
                        }
                    }
                    if (!changed) {
                        // Reset cluster elements as styles/attributes may have changed
                        for (Cluster& cluster : *_clusters) {
                            cluster.clusterElement.reset();
                        }
                        return;
                    }
                }
            }

            // Rebuild clusters, by doing bottom-up merging into a single cluster
            rootClusterIdx = mergeClusters(clusterIdxs.begin(), clusterIdxs.end(), *clusters, *projectionSurface, 1).front();
        }

        // Synchronize cluster data
        std::lock_guard<std::mutex> lock(_clusterMutex);
        std::swap(clusters, _clusters);
        std::swap(projectionSurface, _projectionSurface);
        std::swap(singletonClusterCount, _singletonClusterCount);
        std::swap(rootClusterIdx, _rootClusterIdx);
        _renderClusterIdxs.clear();
    }

    int ClusteredVectorLayer::createSingletonCluster(const std::shared_ptr<VectorElement>& element, std::vector<Cluster>& clusters, const ProjectionSurface& projectionSurface) const {
        MapPos mapPos;
        if (!element->isVisible() || !GetVectorElementPos(element, mapPos)) {
            return -1;
        }
        cglib::vec3<double> pos = projectionSurface.calculatePosition(_dataSource->getProjection()->toInternal(mapPos));

        int clusterIdx = static_cast<int>(clusters.size());
        clusters.emplace_back();
        Cluster& cluster = clusters.back();
        cluster.maxDistance = 0;
        cluster.expandPx = 0;
        cluster.staticPos = cluster.transitionPos = mapPos;
        cluster.bounds = cglib::bbox3<double>(pos, pos);
        cluster.elementCount = 1;
        cluster.childClusterIdx[0] = -1;
        cluster.childClusterIdx[1] = -1;
        cluster.parentClusterIdx = -1;
        cluster.vectorElement = element;
        return clusterIdx;
    }

    int ClusteredVectorLayer::createMergedCluster(int clusterIdx1, int clusterIdx2, std::vector<Cluster>& clusters, const ProjectionSurface& projectionSurface) const {
        int n1 = clusters[clusterIdx1].elementCount;
        int n2 = clusters[clusterIdx2].elementCount;
        const MapPos& clusterPos1 = clusters[clusterIdx1].staticPos;
        const MapPos& clusterPos2 = clusters[clusterIdx2].staticPos;
        MapPos internalPos1 = _dataSource->getProjection()->toInternal(clusterPos1);
        MapPos internalPos2 = _dataSource->getProjection()->toInternal(clusterPos2);
        double dist = projectionSurface.calculateDistance(projectionSurface.calculatePosition(internalPos1), projectionSurface.calculatePosition(internalPos2));
        MapPos mapPos((clusterPos1.getX() * n1 + clusterPos2.getX() * n2) / (n1 + n2), (clusterPos1.getY() * n1 + clusterPos2.getY() * n2) / (n1 + n2));

        int clusterIdx = static_cast<int>(clusters.size());
        clusters.emplace_back();
        Cluster& cluster = clusters.back();
        cluster.maxDistance = dist;
        cluster.expandPx = 0;
        cluster.staticPos = cluster.transitionPos = mapPos;
        cluster.bounds = clusters[clusterIdx1].bounds;
        cluster.bounds.add(clusters[clusterIdx2].bounds);
        cluster.elementCount = n1 + n2;
        cluster.childClusterIdx[0] = clusterIdx1;
        cluster.childClusterIdx[1] = clusterIdx2;
        cluster.parentClusterIdx = -1;
        clusters[clusterIdx1].parentClusterIdx = clusterIdx;
        clusters[clusterIdx2].parentClusterIdx = clusterIdx;
        return clusterIdx;
    }

    std::vector<int> ClusteredVectorLayer::mergeClusters(std::vector<int>::iterator clustersBegin, std::vector<int>::iterator clustersEnd, std::vector<Cluster>& clusters, const ProjectionSurface& projectionSurface, std::size_t maxClusters) const {
        struct ClusterInfo {
            int clusterIdx;
            double closestClusterDistance;
            std::list<ClusterInfo>::iterator closestClusterInfoIt;
        };
        
        std::size_t initialClusters = clustersEnd - clustersBegin;

        // Find axis of bigger variance, use this axis for sorting
        double meanFactor = 1.0 / initialClusters;
        cglib::vec2<double> mean = std::accumulate(clustersBegin, clustersEnd, cglib::vec2<double>::zero(), [meanFactor, &clusters](const cglib::vec2<double>& result, int clusterIdx) {
            const MapPos& pos = clusters[clusterIdx].staticPos;
            return result + cglib::vec2<double>(pos.getX(), pos.getY()) * meanFactor;
        });
        cglib::vec2<double> variance = std::accumulate(clustersBegin, clustersEnd, cglib::vec2<double>::zero(), [mean, &clusters](const cglib::vec2<double>& result, int clusterIdx) {
            const MapPos& pos = clusters[clusterIdx].staticPos;
            cglib::vec2<double> delta = cglib::vec2<double>(pos.getX(), pos.getY()) - mean;
            return result + cglib::vec2<double>(delta(0) * delta(0), delta(1) * delta(1));
        });

        int axis = (variance(0) > variance(1) ? 0 : 1);
        auto clusterComparator = [axis, &clusters](int clusterIdx1, int clusterIdx2) {
            return clusters[clusterIdx1].staticPos[axis] < clusters[clusterIdx2].staticPos[axis];
        };

        // Use hierarchical clustering, if size above threshold
        std::list<ClusterInfo> clusterInfos;
        if (initialClusters > 2 * HIERARCHICAL_MODE_THRESHOLD) {
            std::vector<int> childClusterIdxs1;
            childClusterIdxs1.reserve(initialClusters);
            std::vector<int> childClusterIdxs2;
            childClusterIdxs2.reserve(initialClusters);
            for (auto it = clustersBegin; it != clustersEnd; it++) {
                const Cluster& cluster = clusters[*it];
                if (cluster.staticPos[axis] < mean(axis)) {
                    childClusterIdxs1.push_back(*it);
                } else {
                    childClusterIdxs2.push_back(*it);
                }
            }
            if (!childClusterIdxs1.empty() && !childClusterIdxs2.empty()) {
                childClusterIdxs1 = mergeClusters(childClusterIdxs1.begin(), childClusterIdxs1.end(), clusters, projectionSurface, HIERARCHICAL_MODE_THRESHOLD);
                childClusterIdxs2 = mergeClusters(childClusterIdxs2.begin(), childClusterIdxs2.end(), clusters, projectionSurface, HIERARCHICAL_MODE_THRESHOLD);
                std::sort(childClusterIdxs1.begin(), childClusterIdxs1.end(), clusterComparator);
                std::sort(childClusterIdxs2.begin(), childClusterIdxs2.end(), clusterComparator);
                for (int clusterIdx : childClusterIdxs1) {
                    ClusterInfo clusterInfo;
                    clusterInfo.clusterIdx = clusterIdx;
                    clusterInfos.push_back(clusterInfo);
                }
                for (int clusterIdx : childClusterIdxs2) {
                    ClusterInfo clusterInfo;
                    clusterInfo.clusterIdx = clusterIdx;
                    clusterInfos.push_back(clusterInfo);
                }
            }
        }
        
        // If hierarchical clustering was not needed/did not succeed, use normal sorting/create full clustering
        if (clusterInfos.empty()) {
            std::sort(clustersBegin, clustersEnd, clusterComparator);
            for (auto it = clustersBegin; it != clustersEnd; it++) {
                ClusterInfo clusterInfo;
                clusterInfo.clusterIdx = *it;
                clusterInfos.push_back(clusterInfo);
            }
        }

        // Find closest clusters. n*sqrt(n) steps avg, n^2 worst
        for (auto it = clusterInfos.begin(); it != clusterInfos.end(); it++) {
            it->closestClusterDistance = std::numeric_limits<double>::infinity();
            it->closestClusterInfoIt = clusterInfos.end();
            for (auto testIt = it; ++testIt != clusterInfos.end(); ) {
                if (clusters[testIt->clusterIdx].staticPos[axis] - clusters[it->clusterIdx].staticPos[axis] >= it->closestClusterDistance) {
                    break;
                }
                double distanceSqr = MapVec(clusters[testIt->clusterIdx].staticPos - clusters[it->clusterIdx].staticPos).lengthSqr();
                if (distanceSqr < it->closestClusterDistance * it->closestClusterDistance) {
                    it->closestClusterDistance = std::sqrt(distanceSqr);
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
            mergedClusterInfo.clusterIdx = createMergedCluster(it1->clusterIdx, it2->clusterIdx, clusters, projectionSurface);
            mergedClusterInfo.closestClusterDistance = std::numeric_limits<double>::infinity();
            mergedClusterInfo.closestClusterInfoIt = it2; // to force recalculation

            // Insert cluster to sorted cluster info list. 1 steps avg, n steps worst
            std::list<ClusterInfo>::iterator lastIt = clusterInfos.erase(it2);
            std::list<ClusterInfo>::iterator firstIt = clusterInfos.erase(it1);
            std::list<ClusterInfo>::iterator mergeIt;
            for (mergeIt = firstIt; mergeIt != lastIt; mergeIt++) {
                if (clusters[mergedClusterInfo.clusterIdx].staticPos[axis] > clusters[mergeIt->clusterIdx].staticPos[axis]) {
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
                        if (clusters[testIt->clusterIdx].staticPos[axis] - clusters[it->clusterIdx].staticPos[axis] >= it->closestClusterDistance) {
                            break;
                        }
                        double distanceSqr = MapVec(clusters[testIt->clusterIdx].staticPos - clusters[it->clusterIdx].staticPos).lengthSqr();
                        if (distanceSqr < it->closestClusterDistance * it->closestClusterDistance) {
                            it->closestClusterDistance = std::sqrt(distanceSqr);
                            it->closestClusterInfoIt = testIt;
                        }
                    }
                }
            }

            // Update clusters, check if merged cluster is closer than previously closest cluster. n/2 steps avg
            for (auto it = clusterInfos.begin(); it != mergeIt; it++) {
                double distanceSqr = MapVec(clusters[mergeIt->clusterIdx].staticPos - clusters[it->clusterIdx].staticPos).lengthSqr();
                if (distanceSqr < it->closestClusterDistance * it->closestClusterDistance) {
                    it->closestClusterDistance = std::sqrt(distanceSqr);
                    it->closestClusterInfoIt = mergeIt;
                }
            }
        }

        // Return the remaining clusters
        std::vector<int> clusterIdxs;
        clusterIdxs.reserve(maxClusters);
        for (const ClusterInfo& clusterInfo : clusterInfos) {
            clusterIdxs.push_back(clusterInfo.clusterIdx);
        }
        return clusterIdxs;
    }

    bool ClusteredVectorLayer::renderClusters(const ViewState& viewState, float deltaSeconds) {
        std::lock_guard<std::mutex> lock(_clusterMutex);
        if (viewState.getProjectionSurface() != _projectionSurface) {
            return false;
        }

        // Initialize render state, use previously renderered cluster list
        RenderState renderState;
        renderState.pixelMeasure = viewState.estimateWorldPixelMeasure() * _dpiScale;
        renderState.totalExpanded = 0;
        renderState.expandedClusterIdx = -1;
        renderState.clusters = _clusters;
        renderState.projectionSurface = _projectionSurface;
        renderState.visibleIdxSet.insert(_renderClusterIdxs.begin(), _renderClusterIdxs.end());
        for (int clusterIdx : _renderClusterIdxs) {
            const Cluster& cluster = (*renderState.clusters)[clusterIdx];
            for (int parentClusterIdx = cluster.parentClusterIdx; parentClusterIdx != -1; ) {
                const Cluster& parentCluster = (*renderState.clusters)[parentClusterIdx];
                renderState.visibleChildIdxMap[parentClusterIdx].push_back(clusterIdx);
                parentClusterIdx = parentCluster.parentClusterIdx;
            }
        }

        // Create new rendering list
        _renderClusterIdxs.clear();
        bool refresh = renderCluster(_rootClusterIdx, viewState, renderState, deltaSeconds);
        
        // First pass, create rendering elements from scratch
        for (int clusterIdx : _renderClusterIdxs) {
            Cluster& cluster = (*renderState.clusters)[clusterIdx];
            std::shared_ptr<VectorElement> element;
            if (cluster.elementCount == 1 && cluster.transitionPos == cluster.staticPos) {
                element = cluster.vectorElement;
            } else {
                if (!cluster.clusterElement) {
                    if (_clusterBuilderMode == ClusterBuilderMode::CLUSTER_BUILDER_MODE_ELEMENT_COUNT && cluster.elementCount > 1) {
                        cluster.clusterElement = _clusterElementBuilder->buildClusterElement(cluster.transitionPos, cluster.elementCount); // NOTE: prone to deadlock, but no better way
                    } else {
                        std::vector<std::shared_ptr<VectorElement> > elements;
                        elements.reserve(cluster.elementCount);
                        StoreVectorElements(clusterIdx, *renderState.clusters, elements);
                        cluster.clusterElement = _clusterElementBuilder->buildClusterElement(cluster.transitionPos, elements); // NOTE: prone to deadlock, but no better way
                    }
                }
                element = cluster.clusterElement;
            }
            if (element) {
                addRendererElement(element, viewState);
            }
        }

        bool billboardsChanged = refreshRendererElements();

        // Second pass, update positions of clustering elements
        for (int clusterIdx : _renderClusterIdxs) {
            Cluster& cluster = (*renderState.clusters)[clusterIdx];
            if (cluster.clusterElement && !(cluster.elementCount == 1 && cluster.transitionPos == cluster.staticPos)) {
                SetVectorElementPos(cluster.clusterElement, cluster.transitionPos);
                billboardsChanged = syncRendererElement(cluster.clusterElement, viewState, false) || billboardsChanged;
            }
        }

        // If billboards were changed and no animations running, notify map renderer
        if (billboardsChanged && !refresh) {
            if (auto mapRenderer = getMapRenderer()) {
                mapRenderer->billboardsChanged();
            }
        }
        
        return refresh;
    }

    bool ClusteredVectorLayer::renderCluster(int clusterIdx, const ViewState& viewState, RenderState& renderState, float deltaSeconds) {
        if (clusterIdx == -1) {
            return false;
        }
        const Cluster& cluster = (*renderState.clusters)[clusterIdx];
        if (!viewState.getFrustum().inside(cluster.bounds)) {
            return false;
        }

        // Handle expanded clusters. Expanded clusters are always fully expanded
        bool stop = false;
        if (viewState.getZoom() < _maxClusterZoom) {
            if (renderState.expandedClusterIdx == -1 && cluster.expandPx > 0) {
                moveCluster(clusterIdx, cluster.staticPos, renderState, deltaSeconds);
                renderState.expandedClusterIdx = clusterIdx;
                renderState.totalExpanded = 0;
            } else {
                stop = cluster.maxDistance < _minClusterDistance * renderState.pixelMeasure;
            }
        }
        if (cluster.elementCount == 1 || stop) {
            return animateCluster(clusterIdx, renderState, deltaSeconds);
        }

        // Draw subclusters recursively
        bool refresh = false;
        if (renderCluster(cluster.childClusterIdx[0], viewState, renderState, deltaSeconds)) {
            refresh = true;
        }
        if (renderCluster(cluster.childClusterIdx[1], viewState, renderState, deltaSeconds)) {
            refresh = true;
        }

        // Undo expanded state
        if (renderState.expandedClusterIdx == clusterIdx) {
            renderState.expandedClusterIdx = -1;
        }
        return refresh;
    }

    bool ClusteredVectorLayer::animateCluster(int clusterIdx, RenderState& renderState, float deltaSeconds) {
        if (clusterIdx == -1) {
            return false;
        }
        Cluster& cluster = (*renderState.clusters)[clusterIdx];

        // Transition all visible children from actual position to parent position
        bool refresh = false;
        auto it = renderState.visibleChildIdxMap.find(clusterIdx);
        if (it != renderState.visibleChildIdxMap.end()) {
            for (int childClusterIdx : it->second) {
                if (moveCluster(childClusterIdx, cluster.staticPos, renderState, deltaSeconds)) {
                    refresh = true;
                }
            }
            if (refresh) {
                _renderClusterIdxs.insert(_renderClusterIdxs.end(), it->second.begin(), it->second.end());
                return refresh;
            }
        }

        // Transition from expanded parent to actual expanded positions
        if (renderState.expandedClusterIdx != -1) {
            if (renderState.visibleIdxSet.find(renderState.expandedClusterIdx) != renderState.visibleIdxSet.end()) {
                cluster.transitionPos = (*renderState.clusters)[renderState.expandedClusterIdx].transitionPos;
            }
            if (moveCluster(clusterIdx, createExpandedElementPos(renderState), renderState, deltaSeconds)) {
                refresh = true;
            }
            _renderClusterIdxs.push_back(clusterIdx);
            return refresh;
        }

        // Transition from visible parent to actual position
        for (int parentClusterIdx = clusterIdx; parentClusterIdx != -1; ) {
            Cluster& parentCluster = (*renderState.clusters)[parentClusterIdx];
            if (renderState.visibleIdxSet.find(parentClusterIdx) != renderState.visibleIdxSet.end()) {
                cluster.transitionPos = parentCluster.transitionPos;
                if (moveCluster(clusterIdx, cluster.staticPos, renderState, deltaSeconds)) {
                    refresh = true;
                }
                _renderClusterIdxs.push_back(clusterIdx);
                return refresh;
            }
            parentClusterIdx = parentCluster.parentClusterIdx;
        }

        // New cluster
        cluster.transitionPos = cluster.staticPos;
        _renderClusterIdxs.push_back(clusterIdx);
        return refresh;
    }

    bool ClusteredVectorLayer::moveCluster(int clusterIdx, const MapPos& targetPos, const RenderState& renderState, float deltaSeconds) {
        if (clusterIdx == -1) {
            return false;
        }
        Cluster& cluster = (*renderState.clusters)[clusterIdx];

        bool animated = _animatedClusters;
        if (animated) {
            cglib::vec3<double> pos1 = renderState.projectionSurface->calculatePosition(_dataSource->getProjection()->toInternal(cluster.transitionPos));
            cglib::vec3<double> pos2 = renderState.projectionSurface->calculatePosition(_dataSource->getProjection()->toInternal(targetPos));
            double dist = renderState.projectionSurface->calculateDistance(pos1, pos2);
            if (dist <= renderState.pixelMeasure * 0.25) {
                animated = false;
            }
        }

        if (animated) {
            for (float t = 0; t < 0.05f && t < deltaSeconds; t += 0.0015f) {
                cluster.transitionPos = MapPos(cluster.transitionPos.getX() * 0.99 + targetPos.getX() * 0.01, cluster.transitionPos.getY() * 0.99 + targetPos.getY() * 0.01);
            }
        } else {
            cluster.transitionPos = targetPos;
        }
        return animated;
    }

    MapPos ClusteredVectorLayer::createExpandedElementPos(RenderState& renderState) const {
        Cluster& expandedCluster = (*renderState.clusters)[renderState.expandedClusterIdx];
        MapPos internalPos = _dataSource->getProjection()->toInternal(expandedCluster.transitionPos);
        double angle = Const::PI * 2 * renderState.totalExpanded++ / expandedCluster.elementCount;
        double dist = expandedCluster.expandPx * renderState.pixelMeasure;
        // Note: not uniform across screen, approximation
        return _dataSource->getProjection()->fromInternal(internalPos + MapVec(std::cos(angle), std::sin(angle)) * dist);
    }

    void ClusteredVectorLayer::StoreVectorElements(int clusterIdx, const std::vector<Cluster>& clusters, std::vector<std::shared_ptr<VectorElement> >& elements) {
        if (clusterIdx == -1) {
            return;
        }

        const Cluster& cluster = clusters[clusterIdx];
        if (cluster.vectorElement) {
            elements.push_back(cluster.vectorElement);
        }
        StoreVectorElements(cluster.childClusterIdx[0], clusters, elements);
        StoreVectorElements(cluster.childClusterIdx[1], clusters, elements);
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

    const unsigned int ClusteredVectorLayer::HIERARCHICAL_MODE_THRESHOLD = 100;

}
