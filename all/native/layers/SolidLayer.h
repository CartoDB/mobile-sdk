/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SOLIDLAYER_H_
#define _CARTO_SOLIDLAYER_H_

#include "core/MapEnvelope.h"
#include "components/CancelableTask.h"
#include "components/DirectorPtr.h"
#include "datasources/VectorDataSource.h"
#include "graphics/Color.h"
#include "layers/Layer.h"

#include <atomic>
#include <memory>
#include <unordered_map>

namespace carto {
    class CullState;
    class ViewState;

    class Bitmap;

    class SolidRenderer;
    
    /**
     * A solid layer that displays predefined color or bitmap.
     * @deprecated
     */
    class SolidLayer : public Layer {
    public:
        /**
         * Constructs a SolidLayer object from a color.
         * @param color The color for the layer.
         */
        explicit SolidLayer(const Color& color);
        /**
         * Constructs a SolidLayer object from a bitmap.
         * @param bitmap The bitmap for the layer.
         */
        explicit SolidLayer(const std::shared_ptr<Bitmap>& bitmap);
        virtual ~SolidLayer();
        
        /**
         * Returns the color of this layer.
         * @return The color of this layer..
         */
        Color getColor() const;
        /**
         * Sets the color of this layer.
         * @param color The new color for the layer. Note: if bitmap is defined, the color is multiplied with the bitmap.
         */
        void setColor(const Color& color);
        
        /**
         * Returns the bitmap of this layer.
         * @return The bitmap of this layer. Can be null.
         */
        std::shared_ptr<Bitmap> getBitmap() const;
        /**
         * Sets the bitmap of this layer.
         * @param bitmap The new bitmap for the layer. Can be null, in that case the layer color is used.
         */
        void setBitmap(const std::shared_ptr<Bitmap>& bitmap);
        
        /**
         * Returns the bitmap scaling factor.
         * @return The bitmap scaling factor.
         */
        float getBitmapScale() const;
        /**
         * Sets the bitmap scaling factor.
         * @param scale The new bitmap scaling factor. Default is 1, which corresponds to 1:1 mapping between bitmap pixels and screen pixels.
         */
        void setBitmapScale(float scale);
        
        virtual bool isUpdateInProgress() const;
        
    protected:
        virtual void setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                   const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                   const std::weak_ptr<Options>& options,
                                   const std::weak_ptr<MapRenderer>& mapRenderer,
                                   const std::weak_ptr<TouchHandler>& touchHandler);
        
        virtual void loadData(const std::shared_ptr<CullState>& cullState);

        virtual void offsetLayerHorizontally(double offset);
        
        virtual bool onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState);
        
        virtual std::shared_ptr<Bitmap> getBackgroundBitmap() const;
        virtual std::shared_ptr<Bitmap> getSkyBitmap() const;

        virtual void calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const;
        virtual bool processClick(const ClickInfo& clickInfo, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const;

        virtual void registerDataSourceListener();
        virtual void unregisterDataSourceListener();

    private:
        std::atomic<Color> _color;
        std::shared_ptr<Bitmap> _bitmap;
        std::atomic<float> _bitmapScale;
        std::shared_ptr<SolidRenderer> _solidRenderer;
    };
    
}

#endif
