/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CLUSTEREDVECTORLAYER_H_
#define _CARTO_CLUSTEREDVECTORLAYER_H_

#include "core/MapPos.h"
#include "components/DirectorPtr.h"
#include "graphics/ViewState.h"
#include "layers/VectorLayer.h"
#include "layers/ClusterElementBuilder.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <memory>
#include <utility>
#include <mutex>

namespace carto {
    class VectorElement;
    class LocalVectorDataSource;

    /**
     * A vector layer that supports clustering point-type features.
     * A Pairwise centroid hierarchical clustering algorithm is used internally.
     */
    class ClusteredVectorLayer : public VectorLayer {
    public:
        /**
         * Constructs a new ClusteredVectorLayer instance from data source and a special callback.
         * @param dataSource The local data source that contains the elements for clustering.
         * @param clusterElementBuilder The callback for creating and customizing cluster elements.
         */
        ClusteredVectorLayer(const std::shared_ptr<LocalVectorDataSource>& dataSource, const std::shared_ptr<ClusterElementBuilder>& clusterElementBuilder);
        virtual ~ClusteredVectorLayer();

        /**
         * Returns the current callback used for creating cluster elements.
         * @return The associated callback.
         */
        std::shared_ptr<ClusterElementBuilder> getClusterElementBuilder() const;

        /**
         * Returns the current minimum distance between clusters (in device-independent pixels).
         * @return The current minimum clustering distance.
         */
        float getMinimumClusterDistance() const;
        /**
         * Sets the minimum distance between clusters (in device-independent pixels).
         * @param px The new value in device-independent pixels. The default is 100.
         */
        void setMinimumClusterDistance(float px);

        /**
         * Returns the maximum zoom level when clusters are shown. If zoom level is greater, then
         * clusters are replaced with individual elements. Default is 24.
         * @return The maximum clustering zoom level.
         */
        float getMaximumClusterZoom() const;
        /**
         * Sets the maximum zoom level when clusters are shown. If zoom level is greater, then
         * clusters are replaced with individual elements.
         * @param maxZoom The new maximum clustering zoom level.
         */
        void setMaximumClusterZoom(float maxZoom);

        /**
         * Returns the cluster animation flag value.
         * @return True if animation is enabled, false otherwise.
         */
        bool isAnimatedClusters() const;
        /**
         * Enables or disables the animation of clusters. By default animations are enabled.
         * @param animated The animated flag.
         */
        void setAnimatedClusters(bool animated);

        /**
         * Expands or shrinks the given cluster element. In expanded state,
         * all elements of the cluster are placed at specified distance from the cluster center.
         * @param clusterElement The cluster element to expand or shrink.
         * @param px The distance (in device-independent pixels) between cluster center and expanded elements. Use 0 to shrink already expanded cluster.
         * @return True if the element was found and expanded/shrunk. False otherwise.
         */
        bool expandCluster(const std::shared_ptr<VectorElement>& clusterElement, float px);

        virtual void refresh();

    private:
        struct Cluster {
            double maxDistance;
            float expandPx;
            MapPos staticPos;
            MapPos transitionPos;
            MapBounds mapBoundsInternal;
            int elementCount;
            std::shared_ptr<VectorElement> clusterElement;
            std::shared_ptr<VectorElement> vectorElement;
            int parentClusterIdx;
            int childClusterIdx[2];
        };

        struct RenderState {
            double pixelMeasure;
            int totalExpanded;
            int expandedClusterIdx;
            std::shared_ptr<std::vector<Cluster> > clusters;
            std::unordered_set<int> visibleIdxSet;
            std::unordered_map<int, std::vector<int> > visibleChildIdxMap;
        };

        class ClusterFetchTask : public VectorLayer::FetchTask {
        public:
            ClusterFetchTask(const std::weak_ptr<ClusteredVectorLayer>& layer);

        protected:
            virtual bool loadElements(const std::shared_ptr<CullState>& cullState);
        };

        static const std::size_t THRESHOLD = 100;

        const DirectorPtr<ClusterElementBuilder> _clusterElementBuilder;
        ClusterBuilderMode::ClusterBuilderMode _clusterBuilderMode;

        float _minClusterDistance;
        float _maxClusterZoom;
        bool _animatedClusters;
        float _dpiScale;
        std::shared_ptr<std::vector<Cluster> > _clusters;
        int _singletonClusterCount;
        int _rootClusterIdx;
        std::vector<int> _renderClusterIdxs;
        bool _refreshRootCluster;
        mutable std::mutex _clusterMutex; // for _minClusterDistance, _maxClusterZoom, _dpiScale, _rootClusterIdx, _refreshRootCluster, _renderClusters, _renderClusterIdxs

        virtual bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState);

        virtual void refreshElement(const std::shared_ptr<VectorElement>& element, bool remove);

        virtual std::shared_ptr<CancelableTask> createFetchTask(const std::shared_ptr<CullState>& cullState);

        void rebuildClusters(const std::vector<std::shared_ptr<VectorElement> >& vectorElements);
        int createSingletonCluster(const std::shared_ptr<VectorElement>& element, std::vector<Cluster>& clusters) const;
        int createMergedCluster(int clusterIdx1, int clusterIdx2, std::vector<Cluster>& clusters) const;
        std::vector<int> mergeClusters(std::vector<int>::iterator clustersBegin, std::vector<int>::iterator clustersEnd, std::vector<Cluster>& clusters, std::size_t maxClusters) const;

        bool renderClusters(const ViewState& viewState, float deltaSeconds);
        bool renderCluster(int clusterIdx, const ViewState& viewState, RenderState& renderState, float deltaSeconds);
        bool animateCluster(int clusterIdx, RenderState& renderState, float deltaSeconds);
        bool moveCluster(int clusterIdx, const MapPos& targetPos, const RenderState& renderState, float deltaSeconds);
        MapPos createExpandedElementPos(RenderState& renderState) const;

        static void StoreVectorElements(int clusterIdx, const std::vector<Cluster>& clusters, std::vector<std::shared_ptr<VectorElement> >& elements);
        static bool GetVectorElementPos(const std::shared_ptr<VectorElement>& vectorElement, MapPos& pos);
        static bool SetVectorElementPos(const std::shared_ptr<VectorElement>& vectorElement, const MapPos& pos);
    };

}

#endif
