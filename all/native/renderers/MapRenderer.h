/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPRENDERER_H_
#define _CARTO_MAPRENDERER_H_

#include "components/DirectorPtr.h"
#include "graphics/ViewState.h"
#include "renderers/components/StyleTextureCache.h"
#include "renderers/BackgroundRenderer.h"
#include "renderers/components/AnimationHandler.h"
#include "renderers/components/BillboardSorter.h"
#include "renderers/components/KineticEventHandler.h"
#include "renderers/WatermarkRenderer.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

namespace carto {
    class CameraPanEvent;
    class CameraRotationEvent;
    class CameraTiltEvent;
    class CameraZoomEvent;
    class Bitmap;
    class BillboardDrawData;
    class Layer;
    class Layers;
    class MapPos;
    class ScreenPos;
    class ScreenBounds;
    class MapVec;
    class MapRendererListener;
    class RendererCaptureListener;
    class RedrawRequestListener;
    class RayIntersectedElement;
    class Options;
    class ThreadWorker;
    class CullWorker;
    class BillboardPlacementWorker;
    class RedrawWorker;
    class ShaderManager;
    class TextureManager;

    /**
     * The map renderer component.
     */
    class MapRenderer : public std::enable_shared_from_this<MapRenderer> {
    public:
        struct OnChangeListener {
            virtual ~OnChangeListener() { }
            
            virtual void onMapChanged() = 0;
            virtual void onMapIdle() = 0;
        };

        MapRenderer(const std::shared_ptr<Layers>& layers,
                    const std::shared_ptr<Options>& options);
        virtual ~MapRenderer();

        void init();
        void deinit();

        std::shared_ptr<RedrawRequestListener> getRedrawRequestListener() const;
        void setRedrawRequestListener(const std::shared_ptr<RedrawRequestListener>& listener);

        /**
         * Returns the map renderer listener. Can be null.
         * @return The map renderer listener.
         */
        std::shared_ptr<MapRendererListener> getMapRendererListener() const;
        /**
         * Sets the map renderer listener.
         * @param listener The new map renderer listener. Can be null.
         */
        void setMapRendererListener(const std::shared_ptr<MapRendererListener>& listener);
        
        /**
         * Returns the current view state.
         * @return The current view state.
         */
        ViewState getViewState() const;
    
        /**
         * Calculates the map position corresponding to a screen position, using the specified view state.
         * @param screenPos The screen position.
         * @param viewState The view state to use.
         * @return The calculated map position in base projection coordinate system.
         */
        MapPos screenToMap(const ScreenPos& screenPos, const ViewState& viewState);
        /**
         * Calculates the screen position corresponding to a map position, using the specified view state.
         * @param mapPos The map position in base projection coordinate system.
         * @param viewState The view state to use.
         * @return The calculated screen position.
         */
        ScreenPos mapToScreen(const MapPos& mapPos, const ViewState& viewState);

        /**
         * Requests the renderer to refresh the view.
         * Note that there is normally no need to do this manually,
         * SDK automatically redraws the view when needed.
         */
        void requestRedraw() const;
    
        /**
         * Captures map rendering as a bitmap. This operation is asynchronous and the result is returned via listener callback.
         * @param listener The listener interface that will receive the callback once rendering is available.
         * @param waitWhileUpdating If true, delay the capture until all asynchronous processes are finished (for example, until all tiles are loaded).
         */
        void captureRendering(const std::shared_ptr<RendererCaptureListener>& listener, bool waitWhileUpdating);
        
        std::vector<std::shared_ptr<BillboardDrawData> > getBillboardDrawDatas() const;
    
        MapPos getCameraPos() const;
        MapPos getFocusPos() const;
        MapVec getUpVec() const;
        float getRotation() const;
        float getTilt() const;
        float getZoom() const;
    
        AnimationHandler& getAnimationHandler();
        KineticEventHandler& getKineticEventHandler();
        
        void calculateCameraEvent(CameraPanEvent& cameraEvent, float durationSeconds, bool updateKinetic);
        void calculateCameraEvent(CameraRotationEvent& cameraEvent, float durationSeconds, bool updateKinetic);
        void calculateCameraEvent(CameraTiltEvent& cameraEvent, float durationSeconds, bool updateKinetic);
        void calculateCameraEvent(CameraZoomEvent& cameraEvent, float durationSeconds, bool updateKinetic);
    
        void moveToFitPoints(const MapPos& center, const std::vector<MapPos>& points, const ScreenBounds& screenBounds, bool integerZoom, bool resetTilt, bool resetRotation, float durationSeconds);
        
        MapPos screenToWorld(const ScreenPos& screenPos, const ViewState& viewState) const;
        ScreenPos worldToScreen(const MapPos& worldPos, const ViewState& viewState) const;
    
        void onSurfaceCreated();
        void onSurfaceChanged(int width, int height);
        void onDrawFrame();
        void onSurfaceDestroyed();
    
        void calculateRayIntersectedElements(const MapPos& targetPos, ViewState& viewState, std::vector<RayIntersectedElement>& results);
    
        void billboardsChanged();
        void layerChanged(const std::shared_ptr<Layer>& layer, bool delay);
        void viewChanged(bool delay);
    
        void registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener);
        void unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener);
        
        void addRenderThreadCallback(const std::shared_ptr<ThreadWorker>& callback);
        
    private:
        class OptionsListener : public Options::OnChangeListener {
        public:
            OptionsListener(const std::shared_ptr<MapRenderer>& mapRenderer);
            
            virtual void onOptionChanged(const std::string& optionName);
            
        private:
            std::weak_ptr<MapRenderer> _mapRenderer;
        };

        void setUpGLState() const;
    
        void drawLayers(float deltaSeconds, const ViewState& viewState);
        
        void handleRenderThreadCallbacks();
        void handleRenderCaptureCallbacks();
    
        static const int BILLBOARD_PLACEMENT_TASK_DELAY;

        static const int STYLE_TEXTURE_CACHE_SIZE; // Size limit (in bytes) for style texture cache
        
        std::chrono::steady_clock::time_point _lastFrameTime;
    
        ViewState _viewState;
    
        std::shared_ptr<ShaderManager> _shaderManager;
        std::shared_ptr<TextureManager> _textureManager;
        std::shared_ptr<StyleTextureCache> _styleCache;
    
        std::shared_ptr<CullWorker> _cullWorker;
        std::thread _cullThread;
#ifdef TARGET_XAMARIN
        std::shared_ptr<RedrawWorker> _redrawWorker;
        std::thread _redrawThread;
#endif
        
        std::shared_ptr<OptionsListener> _optionsListener;
        
        BackgroundRenderer _backgroundRenderer;
        WatermarkRenderer _watermarkRenderer;
        
        BillboardSorter _billboardSorter;
        std::vector<std::shared_ptr<BillboardDrawData> > _billboardDrawDataBuffer;
    
        bool _billboardsChanged;
        std::shared_ptr<BillboardPlacementWorker> _billboardPlacementWorker;
        std::thread _billboardPlacementThread;
    
        AnimationHandler _animationHandler;
        KineticEventHandler _kineticEventHandler;
        
        const std::shared_ptr<Layers> _layers;
        const std::shared_ptr<Options> _options;
        
        bool _surfaceChanged;
        mutable std::atomic<bool> _redrawPending;

        ThreadSafeDirectorPtr<RedrawRequestListener> _redrawRequestListener;

        ThreadSafeDirectorPtr<MapRendererListener> _mapRendererListener;

        std::vector<std::pair<DirectorPtr<RendererCaptureListener>, bool> > _rendererCaptureListeners;
        mutable std::mutex _rendererCaptureListenersMutex;

        std::vector<std::shared_ptr<OnChangeListener> > _onChangeListeners;
        mutable std::mutex _onChangeListenersMutex;

        std::vector<std::shared_ptr<ThreadWorker> > _renderThreadCallbacks;
        mutable std::mutex _renderThreadCallbacksMutex;
    
        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
