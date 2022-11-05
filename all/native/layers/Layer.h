/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LAYER_H_
#define _CARTO_LAYER_H_

#include "core/ScreenPos.h"
#include "core/MapRange.h"
#include "core/Variant.h"
#include "renderers/components/CullState.h"
#include "ui/ClickInfo.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <string>

#include <cglib/ray.h>

namespace carto {
    class Bitmap;
    class BillboardSorter;
    class DataSource;
    class Layers;
    class MapRenderer;
    class TouchHandler;
    class CancelableThreadPool;
    class RayIntersectedElement;
    class ViewState;
    
    /**
     * An abstract base class for all layers.
     */
    class Layer : public std::enable_shared_from_this<Layer> {
    public:
        virtual ~Layer();
    
        /**
         * Returns a copy of the layer meta data map. The changes you make to this map are NOT reflected in the actual meta data of the layer.
         * @return A copy of the layer meta data map.
         */
        std::map<std::string, Variant> getMetaData() const;
        /**
         * Sets a new meta data map for the layer. Old meta data values will be lost.
         * @param metaData The new meta data map for this layer.
         */
        void setMetaData(const std::map<std::string, Variant>& metaData);
        
        /** 
         * Returns true if the specified key exists in the layer meta data map.
         * @param key The key to check.
         * @return True if the meta data element exists.
         */
        bool containsMetaDataKey(const std::string& key) const;
        /** 
         * Returns a layer meta data element corresponding to the key. If no value is found null variant is returned.
         * @param key The key to use.
         * @return The value corresponding to the key from the meta data map. If the key does not exist, empty variant is returned.
         */
        Variant getMetaDataElement(const std::string& key) const;
        /**
         * Adds a new key-value pair to the layer meta data map. If the key already exists in the map,
         * it's value will be replaced by the new value.
         * @param key The new key.
         * @param element The new value.
         */
        void setMetaDataElement(const std::string& key, const Variant& element);
    
        /**
         * Returns the layer task priority of this layer.
         * @return The priority level for the tasks of this layer.
         */
        int getUpdatePriority() const;
        /**
         * Sets the layer task priority. Higher priority layers get to load data before
         * lower priority layers. Normal layers and tile layers have seperate task queues and thus 
         * don't compete with each other for task queue access. The default is 0.
         * @param priority The new task priority for this layer, higher values get better access.
         */
        void setUpdatePriority(int priority);
    
        /**
         * Returns the culling delay of the layer in milliseconds.
         * @return The culling delay in milliseconds.
         */
        int getCullDelay() const;
        /**
         * Sets the layer culling delay. The culling delay is used to delay layer content rendering in case of user interaction,
         * higher delay improves performance and battery life at the expense of interactivity. Default is 200ms-400ms, depending
         * on layer type.
         * @param delay The new culling delay in milliseconds.
         */
        void setCullDelay(int delay);
    
        /**
         * Returns the opacity of this layer.
         * @return The opacity of this layer.
         */
        float getOpacity() const;
        /**
         * Set the opacity of the layer.
         * @param opacity The opacity of the layer in range (0..1). 1.0 is the default value.
         */
        void setOpacity(float opacity);

        /**
         * Returns the visibility of this layer.
         * @return True if the layer is visible.
         */
        bool isVisible() const;
        /**
         * Sets the visibility of this layer.
         * @param visible The new visibility state of the layer.
         */
        void setVisible(bool visible);
    
        /**
         * Returns the visible zoom range of this layer.
         * @return The visible zoom range of this layer.
         */
        MapRange getVisibleZoomRange();
        /**
         * Sets the visible zoom range for this layer. Current zoom level must be within this range for the layer to be visible.
         * This range is half-open, thus layer is visible if range.min <= ZOOMLEVEL < range.max.
         * @param range new visible zoom range
         */
        void setVisibleZoomRange(const MapRange& range);
        
        /**
         * Tests whether this layer is being currently updated.
         * @return True when the layer is being updated or false when the layer is in steady state.
         */
        virtual bool isUpdateInProgress() const = 0;

        /**
         * Updates the layer using new visibility information. This method is periodically called when the map view moves.
         * The visibilty info is saved, so the data can be refreshed later.
         * @param cullState The new visibilty information.
         */
        void update(const std::shared_ptr<CullState>& cullState);
        /**
         * Refreshes the layer using old stored visibility information. This method might be called if some of the layer data
         * changes.
         */
        virtual void refresh();

        /**
         * Simulate click on this layer. This may trigger any event listeners attached to the layer.
         * @param clickType The type of the click.
         * @param screenPos The screen position for the simulated click.
         * @param viewState The view state to use.
         */
        virtual void simulateClick(ClickType::ClickType clickType, const ScreenPos& screenPos, const ViewState& viewState);
    
    protected:
        friend class Layers;
        friend class MapRenderer;
        friend class BackgroundRenderer;
        friend class TouchHandler;
    
        Layer();
        
        virtual void setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                   const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                   const std::weak_ptr<Options>& options,
                                   const std::weak_ptr<MapRenderer>& mapRenderer,
                                   const std::weak_ptr<TouchHandler>& touchHandler);
    
        std::shared_ptr<Options> getOptions() const;
        std::shared_ptr<MapRenderer> getMapRenderer() const;
        std::shared_ptr<TouchHandler> getTouchHandler() const;
        std::shared_ptr<CullState> getLastCullState() const;

        void redraw() const;
    
        virtual void loadData(const std::shared_ptr<CullState>& cullState) = 0;
        
        virtual void offsetLayerHorizontally(double offset) = 0;

        virtual bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState) = 0;
        virtual bool onDrawFrame3D(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState);
        
        virtual std::shared_ptr<Bitmap> getBackgroundBitmap(const ViewState& viewState) const;
        virtual std::shared_ptr<Bitmap> getSkyBitmap(const ViewState& viewState) const;
        
        virtual void calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const = 0;
        virtual bool processClick(const ClickInfo& clickInfo, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const = 0;
    
        virtual void registerDataSourceListener() = 0;
        virtual void unregisterDataSourceListener() = 0;
    
        std::shared_ptr<CancelableThreadPool> _envelopeThreadPool;
        std::shared_ptr<CancelableThreadPool> _tileThreadPool;
        
        mutable std::recursive_mutex _mutex;

    private:
        static const int DEFAULT_CULL_DELAY;

        std::atomic<int> _updatePriority;

        std::atomic<int> _cullDelay;
        
        std::atomic<float> _opacity;
        
        std::atomic<bool> _visible;
        
        MapRange _visibleZoomRange;

        std::map<std::string, Variant> _metaData;

        std::shared_ptr<CullState> _lastCullState;
       
        std::weak_ptr<Options> _options;
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::weak_ptr<TouchHandler> _touchHandler;
    };
    
}

#endif
