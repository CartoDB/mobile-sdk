/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LAYERS_H_
#define _CARTO_LAYERS_H_

#include <memory>
#include <mutex>
#include <vector>

namespace carto {
    class Options;
    class Layer;
    class MapRenderer;
    class TouchHandler;
    class CancelableThreadPool;
    
    /**
     * Container for all raster and vector layers of the map view.
     *
     * The order in which layers are added is important for vector elements like Points, Lines
     * and Polygons. For these elements the layer ordering defines their draw order.
     * Other elements like NMLModels and Polygon3Ds are z ordered and are drawn using the depth buffer.
     * For Billboard elements like Markers and Labels, the layer ordering is unimportant, because
     * they will be sorted from back to front and drawn in that order on top of all other vector elements.
     */
    class Layers {
    public:
        Layers(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
               const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
               const std::weak_ptr<Options>& options);
        virtual ~Layers();
        
        /**
         * Returns the current layer count.
         * @return The layer count.
         */
        int count() const;
        
        /**
         * Clears the layer stack.
         */
        void clear();

        /**
         * Returns the layer at the specified index.
         * @param index The layer index to return. Must be between 0 and count (exclusive).
         * @return The layer at the specified index.
         * @throws std::out_of_range If the index is out of range.
         */
        std::shared_ptr<Layer> get(int index) const;
        /**
         * Returns the list of all layers. The layers are in the order in which they were added.
         * @return A vector of all previously added layers.
         */
        std::vector<std::shared_ptr<Layer> > getAll() const;

        /**
         * Replaces the layer at the specified index.
         * @param index The layer index to replace. Must be between 0 and count (exclusive).
         * @param layer The new layer.
         * @throws std::out_of_range If the index is out of range.
         */
        void set(int index, const std::shared_ptr<Layer>& layer);
        /**
         * Replaces all the layers with the given layer list.
         * @param layers The new list of layers.
         */
        void setAll(const std::vector<std::shared_ptr<Layer> >& layers);
        
        /**
         * Inserts a new layer at the specified position.
         * All previous layers starting from this index will be moved to the next position.
         * @param index The layer index. Must be between 0 and count (inclusive).
         * @param layer The new layer.
         * @throws std::out_of_range If the index is out of range.
         */
        void insert(int index, const std::shared_ptr<Layer>& layer);

        /**
         * Adds a new layer to the layer stack. The new layer will be the last (and topmost) layer.
         * @param layer The layer to be added.
         */
        void add(const std::shared_ptr<Layer>& layer);
        /**
         * Adds a a list of layers to the layer stack. The new layers will be the last (and topmost) layers.
         * @param layers The layer list to be added.
         */
        void addAll(const std::vector<std::shared_ptr<Layer> >& layers);

        /**
         * Removes a layer from the layer stack.
         * @param layer The layer to be removed.
         * @return True if the layer was removed. False otherwise (layer was not found).
         */
        bool remove(const std::shared_ptr<Layer>& layer);
        /**
         * Removes a list of layers from the layer stack.
         * @param layers The list of layers to be removed.
         * @return True if all layer were removed. False otherwise (some layers were not found).
         */
        bool removeAll(const std::vector<std::shared_ptr<Layer> >& layers);

    protected:
        friend class BaseMapView;
        
        void setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::weak_ptr<TouchHandler>& touchHandler);
    
    private:
        std::vector<std::shared_ptr<Layer> > _layers;
    
        std::shared_ptr<CancelableThreadPool> _envelopeThreadPool;
        std::shared_ptr<CancelableThreadPool> _tileThreadPool;
        std::weak_ptr<Options> _options;
        
        std::weak_ptr<MapRenderer> _mapRenderer;
        std::weak_ptr<TouchHandler> _touchHandler;
    
        mutable std::mutex _mutex;
    };
    
}

#endif
