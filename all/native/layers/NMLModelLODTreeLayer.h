/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELLODTREELAYER_H_
#define _CARTO_NMLMODELLODTREELAYER_H_

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "components/CancelableTask.h"
#include "components/CancelableThreadPool.h"
#include "components/DirectorPtr.h"
#include "datasources/NMLModelLODTreeDataSource.h"
#include "graphics/ViewState.h"
#include "layers/Layer.h"

#include <string>
#include <memory>
#include <map>
#include <vector>

#include <stdext/timed_lru_cache.h>

namespace carto {
    class CullState;
    class NMLModelLODTreeDrawData;
    class NMLModelLODTreeRenderer;
    class NMLModelLODTreeEventListener;

    namespace nml {
        class GLModel;
        class GLMesh;
        class GLTexture;
    }
    
    /**
     * An advanced layer for 3D models that supports automatic Level of Detail (LOD) calculation based on view.
     * Should be used together with corresponding data source.
     */
    class NMLModelLODTreeLayer : public Layer {
    public:
        /**
         * Constructs a NMLModelLODTreeLayer object from a data source.
         * @param dataSource The data source from which this layer loads data.
         */
        explicit NMLModelLODTreeLayer(const std::shared_ptr<NMLModelLODTreeDataSource>& dataSource);
        virtual ~NMLModelLODTreeLayer();
    
        /**
         * Returns the data source of this layer.
         * @return The data source that was bound to this vector layer on construction.
         */
        std::shared_ptr<NMLModelLODTreeDataSource> getDataSource() const;

        /**
         * Returns memory usage constraints for the layer.
         * @return The memory usage constraints for the layer.
         */
        std::size_t getMaxMemorySize() const;
        /**
         * Set memory usage constraints for the layer. The specified limit is not exact, 
         * but should be relatively close to the actual memory usage of the layer.
         * If specific view requires more data than specified limit, then lower LOD levels
         * of the models are used. The default is 40MB.
         * @param size The memory limit in bytes.
         */
        void setMaxMemorySize(std::size_t size);

        /**
         * Returns relative model LOD resolution.
         * @return The relative model LOD resolution.
         */
        float getLODResolutionFactor() const;
        /**
         * Set relative model LOD resolution. Higher values than 1 result in higher details 
         * (but slower performance and higher memory usage), while lower values give better 
         * performance but lower quality. The default is 1.
         * @param factor The relative LOD resolution factor.
         */
        void setLODResolutionFactor(float factor);
    
        /**
         * Returns the NML model event listener.
         * @return The NML model event listener.
         */
        std::shared_ptr<NMLModelLODTreeEventListener> getNMLModelLODTreeEventListener() const;
        /**
         * Sets the NML model event listener.
         * @param nmlModelLODTreeEventListener The vector element event listener.
         */
        void setNMLModelLODTreeEventListener(const std::shared_ptr<NMLModelLODTreeEventListener>& nmlModelLODTreeEventListener);
    
        virtual bool isUpdateInProgress() const;

    protected:
        virtual void offsetLayerHorizontally(double offset);
    
        virtual void onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager);
        virtual bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState);
        virtual void onSurfaceDestroyed();
    
        virtual void calculateRayIntersectedElements(const Projection& projection, const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
        virtual bool processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const;
    
        virtual void registerDataSourceListener();
        virtual void unregisterDataSourceListener();
    
        virtual void setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                   const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                   const std::weak_ptr<Options>& options,
                                   const std::weak_ptr<MapRenderer>& mapRenderer,
                                   const std::weak_ptr<TouchHandler>& touchHandler);

        virtual void loadData(const std::shared_ptr<CullState>& cullState);
    
    private:
        typedef std::vector<NMLModelLODTreeDataSource::MapTile> MapTileList;
        typedef std::map<long long, std::shared_ptr<NMLModelLODTree> > ModelLODTreeMap;
        typedef cache::timed_lru_cache<long long, std::shared_ptr<NMLModelLODTree> > ModelLODTreeCache;
        typedef std::map<long long, std::shared_ptr<nml::GLMesh> > MeshMap;
        typedef cache::timed_lru_cache<long long, std::shared_ptr<nml::GLMesh> > MeshCache;
        typedef std::map<long long, std::shared_ptr<nml::GLTexture> > TextureMap;
        typedef cache::timed_lru_cache<long long, std::shared_ptr<nml::GLTexture> > TextureCache;
        typedef std::map<long long, std::shared_ptr<NMLModelLODTreeDrawData> > NodeDrawDataMap;
    
        class FetchingTasks {
        public:
            FetchingTasks() : _fetchingTasks(), _mutex() { }
            
            int getTaskCount() const {
                std::lock_guard<std::mutex> lock(_mutex);
                return static_cast<int>(_fetchingTasks.size());
            }
            
            void add(long long taskId) {
                std::lock_guard<std::mutex> lock(_mutex);
                _fetchingTasks.insert(taskId);
            }
            
            bool exists(long long taskId) {
                std::lock_guard<std::mutex> lock(_mutex);
                return _fetchingTasks.find(taskId) != _fetchingTasks.end();
            }
            
            void remove(long long taskId) {
                std::lock_guard<std::mutex> lock(_mutex);
                _fetchingTasks.erase(taskId);
            }
        private:
            std::unordered_set<long long> _fetchingTasks;
            mutable std::mutex _mutex;
        };
    
        class MapTilesFetchTask : public CancelableTask {
        public:
            MapTilesFetchTask(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const std::shared_ptr<CullState>& cullState);
            virtual void run();
    
        private:
            std::weak_ptr<NMLModelLODTreeLayer> _layer;
            std::shared_ptr<CullState> _cullState;
        };
    
        class ModelLODTreeFetchTask : public CancelableTask {
        public:
            ModelLODTreeFetchTask(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const NMLModelLODTreeDataSource::MapTile& mapTile);
            virtual void cancel();
            virtual void run();
    
        private:
            std::weak_ptr<NMLModelLODTreeLayer> _layer;
            NMLModelLODTreeDataSource::MapTile _mapTile;
        };
    
        class MeshFetchTask : public CancelableTask {
        public:
            MeshFetchTask(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const NMLModelLODTree::MeshBinding& binding);
            virtual void cancel();
            virtual void run();
    
        private:
            std::weak_ptr<NMLModelLODTreeLayer> _layer;
            NMLModelLODTree::MeshBinding _binding;
        };
    
        class TextureFetchTask : public CancelableTask {
        public:
            TextureFetchTask(const std::shared_ptr<NMLModelLODTreeLayer>& layer, const NMLModelLODTree::TextureBinding& binding);
            virtual void cancel();
            virtual void run();
    
        private:
            std::weak_ptr<NMLModelLODTreeLayer> _layer;
            NMLModelLODTree::TextureBinding _binding;
        };
    
        bool isDataAvailable(const NMLModelLODTree* modelLODTree, int nodeId);
        bool loadModelLODTrees(const MapTileList& mapTileList, bool checkOnly);
        bool loadMeshes(const NMLModelLODTree* modelLODTree, int nodeId, bool checkOnly);
        bool loadTextures(const NMLModelLODTree* modelLODTree, int nodeId, bool checkOnly);
        void updateModelLODTrees(const MapTileList& mapTileList, ModelLODTreeMap& modelLODTreeMap);
        void updateMeshes(const NMLModelLODTree* modelLODTree, int nodeId, std::shared_ptr<nml::GLModel> glModel, MeshMap& meshMap);
        void updateTextures(const NMLModelLODTree* modelLODTree, int nodeId, std::shared_ptr<nml::GLModel> glModel, TextureMap& textureMap);
        void updateDrawLists(const ViewState& viewState, MeshMap& meshMap, TextureMap& textureMap, NodeDrawDataMap& nodeDrawDataMap);
    
        static const int MODELLODTREE_LOADING_PRIORITY_OFFSET = 1;
        static const int MESH_LOADING_PRIORITY_OFFSET = 0;
        static const int TEXTURE_LOADING_PRIORITY_OFFSET = 0;

        static const int DEFAULT_MODELLODTREE_CACHE_SIZE = 64;
        static const int DEFAULT_MAX_MEMORY_SIZE = 40 * 1024 * 1024;
        static const int DEFAULT_MESH_CACHE_SIZE = 40 * 1024 * 1024;
        static const int DEFAULT_TEXTURE_CACHE_SIZE = 40 * 1024 * 1024;
    
        std::size_t _maxMemorySize;
        float _LODResolutionFactor;
    
        MapTileList _mapTileList;
        ViewState _mapTileListViewState;
        ModelLODTreeMap _modelLODTreeMap;
        ModelLODTreeCache _modelLODTreeCache;
        MeshMap _meshMap;
        MeshCache _meshCache;
        TextureMap _textureMap;
        TextureCache _textureCache;
        NodeDrawDataMap _nodeDrawDataMap;
    
        FetchingTasks _fetchingModelLODTrees;
        FetchingTasks _fetchingMeshes;
        FetchingTasks _fetchingTextures;
        
        std::shared_ptr<CancelableThreadPool> _fetchThreadPool;

        ThreadSafeDirectorPtr<NMLModelLODTreeEventListener> _nmlModelLODTreeEventListener;
    
        std::shared_ptr<NMLModelLODTreeDataSource> _dataSource;
        std::shared_ptr<NMLModelLODTreeRenderer> _renderer;
    };
    
}

#endif

#endif
