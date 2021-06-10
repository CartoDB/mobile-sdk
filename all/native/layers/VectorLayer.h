/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORLAYER_H_
#define _CARTO_VECTORLAYER_H_

#include "components/CancelableTask.h"
#include "components/DirectorPtr.h"
#include "core/MapEnvelope.h"
#include "datasources/VectorDataSource.h"
#include "layers/Layer.h"

#include <memory>
#include <mutex>

namespace carto {
    class CullState;
    class ViewState;

    class Billboard;
    class GeometryCollection;
    class Label;
    class Line;
    class LineStyle;
    class Marker;
    class NMLModel;
    class Point;
    class Polygon3D;
    class Polygon;
    class Popup;
    class VectorElementEventListener;
    
    class BillboardRenderer;
    class BillboardSorter;
    class GeometryCollectionRenderer;
    class LineRenderer;
    class PointRenderer;
    class Polygon3DRenderer;
    class PolygonRenderer;
    
    /**
     * A vector layer that loads data using an envelope. Should be used together with corresponding data source.
     */
    class VectorLayer : public Layer {
    public:
        /**
         * Constructs a VectorLayer object from a data source.
         * @param dataSource The data source from which this layer loads data.
         */
        explicit VectorLayer(const std::shared_ptr<VectorDataSource>& dataSource);
        virtual ~VectorLayer();
        
        /**
         * Returns the vector data source of this vector layer.
         * @return The vector data source that was bound to this vector layer on construction.
         */
        std::shared_ptr<VectorDataSource> getDataSource() const;
        
        /**
         * Returns the vector element event listener.
         * @return The vector element event listener.
         */
        std::shared_ptr<VectorElementEventListener> getVectorElementEventListener() const;
        /**
         * Sets the vector element event listener.
         * @param eventListener The vector element event listener.
         */
        void setVectorElementEventListener(const std::shared_ptr<VectorElementEventListener>& eventListener);

        /**
         * Returns true if Z-buffering is enabled for 2D geometry. By default it is disabled and used only for billboards.
         * @return True if Z-buffering is enabled for 2D geometry.
         */
        bool isZBuffering() const;
        /**
         * Sets the Z-buffering flag for 2D geometry. By default it is disabled.
         * @param enabled True if Z-buffering should be enabled.
         */
        void setZBuffering(bool enabled);
    
        virtual bool isUpdateInProgress() const;
        
    protected:
        class DataSourceListener : public VectorDataSource::OnChangeListener {
        public:
            explicit DataSourceListener(const std::shared_ptr<VectorLayer>& layer);
            
            virtual void onElementAdded(const std::shared_ptr<VectorElement>& element);
            virtual void onElementChanged(const std::shared_ptr<VectorElement>& element);
            virtual void onElementRemoved(const std::shared_ptr<VectorElement>& element);
            virtual void onElementsAdded(const std::vector<std::shared_ptr<VectorElement> >& elements);
            virtual void onElementsChanged();
            virtual void onElementsRemoved();
            
        private:
            std::weak_ptr<VectorLayer> _layer;
        };
        
        class FetchTask : public CancelableTask {
        public:
            explicit FetchTask(const std::weak_ptr<VectorLayer>& layer);

            virtual void cancel();
            virtual void run();
            
        protected:
            std::weak_ptr<VectorLayer> _layer;
            
            bool _started;

            virtual bool loadElements(const std::shared_ptr<VectorLayer>& layer, const std::shared_ptr<CullState>& cullState);
        };
        
        class FetchingTasks {
        public:
            FetchingTasks() : _fetchingTasks(), _mutex() { }
            
            void insert(const std::shared_ptr<FetchTask>& task) {
                std::lock_guard<std::mutex> lock(_mutex);
                auto it = std::find(_fetchingTasks.begin(), _fetchingTasks.end(), task);
                if (it == _fetchingTasks.end()) {
                    _fetchingTasks.push_back(task);
                }
            }
            
            void remove(const std::shared_ptr<FetchTask>& task) {
                std::lock_guard<std::mutex> lock(_mutex);
                auto it = std::find(_fetchingTasks.begin(), _fetchingTasks.end(), task);
                if (it != _fetchingTasks.end()) {
                    _fetchingTasks.erase(it);
                }
            }

            std::shared_ptr<FetchTask> getLast() const {
                std::lock_guard<std::mutex> lock(_mutex);
                if (!_fetchingTasks.empty()) {
                    return _fetchingTasks.back();
                }
                return std::shared_ptr<FetchTask>();
            }
            
            std::vector<std::shared_ptr<FetchTask> > getAll() const {
                std::lock_guard<std::mutex> lock(_mutex);
                return _fetchingTasks;
            }

            int getCount() const {
                std::lock_guard<std::mutex> lock(_mutex);
                return static_cast<int>(_fetchingTasks.size());
            }

        private:
            std::vector<std::shared_ptr<FetchTask> > _fetchingTasks;
            mutable std::mutex _mutex;
        };

        virtual void setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                   const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                   const std::weak_ptr<Options>& options,
                                   const std::weak_ptr<MapRenderer>& mapRenderer,
                                   const std::weak_ptr<TouchHandler>& touchHandler);
        
        virtual void loadData(const std::shared_ptr<CullState>& cullState);

        virtual void offsetLayerHorizontally(double offset);
        
        virtual bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState);
        
        virtual void calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
        virtual bool processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const;

        virtual void refreshElement(const std::shared_ptr<VectorElement>& element, bool remove);

        virtual void addRendererElement(const std::shared_ptr<VectorElement>& element, const ViewState& viewState);
        virtual bool refreshRendererElements();
        virtual bool syncRendererElement(const std::shared_ptr<VectorElement>& element, const ViewState& viewState, bool remove);
        
        virtual void registerDataSourceListener();
        virtual void unregisterDataSourceListener();

        virtual std::shared_ptr<FetchTask> createFetchTask(const std::shared_ptr<CullState>& cullState);

        const DirectorPtr<VectorDataSource> _dataSource;
        std::shared_ptr<VectorDataSource::OnChangeListener> _dataSourceListener;
        
        std::atomic<bool> _zBuffering;

    private:
        ThreadSafeDirectorPtr<VectorElementEventListener> _vectorElementEventListener;

        std::shared_ptr<BillboardRenderer> _billboardRenderer;
        std::shared_ptr<GeometryCollectionRenderer> _geometryCollectionRenderer;
        std::shared_ptr<LineRenderer> _lineRenderer;
        std::shared_ptr<PointRenderer> _pointRenderer;
        std::shared_ptr<PolygonRenderer> _polygonRenderer;
        std::shared_ptr<Polygon3DRenderer> _polygon3DRenderer;
    
        FetchingTasks _fetchingTasks;
    };
    
}

#endif
