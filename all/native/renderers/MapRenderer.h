/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPRENDERER_H_
#define _CARTO_MAPRENDERER_H_

#include "core/MapPos.h"
#include "core/MapVec.h"
#include "core/ScreenPos.h"
#include "core/ScreenBounds.h"
#include "components/DirectorPtr.h"
#include "graphics/ViewState.h"
#include "renderers/BackgroundRenderer.h"
#include "renderers/components/AnimationHandler.h"
#include "renderers/components/KineticEventHandler.h"
#include "renderers/WatermarkRenderer.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>
#include <map>

namespace carto {
    class CameraPanEvent;
    class CameraRotationEvent;
    class CameraTiltEvent;
    class CameraZoomEvent;
    class Bitmap;
    class BillboardDrawData;
    class Layer;
    class Layers;
    class MapRendererListener;
    class RendererCaptureListener;
    class RedrawRequestListener;
    class RayIntersectedElement;
    class Options;
    class ThreadWorker;
    class CullWorker;
    class VTLabelPlacementWorker;
    class BillboardPlacementWorker;
    class FrameBuffer;
    class Shader;
    class Texture;
    class GLResourceManager;

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

        MapRenderer(const std::shared_ptr<Layers>& layers, const std::shared_ptr<Options>& options);
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
         * Returns the current projectin surface object.
         * @return The current projection surface object.
         */
        std::shared_ptr<ProjectionSurface> getProjectionSurface() const;
    
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

        std::shared_ptr<Layers> getLayers() const;
        
        std::shared_ptr<GLResourceManager> getGLResourceManager() const;

        std::vector<std::shared_ptr<BillboardDrawData> > getBillboardDrawDatas() const;
    
        AnimationHandler& getAnimationHandler();
        KineticEventHandler& getKineticEventHandler();

        void calculateCameraEvent(CameraPanEvent& cameraEvent, float durationSeconds, bool updateKinetic);
        void calculateCameraEvent(CameraRotationEvent& cameraEvent, float durationSeconds, bool updateKinetic);
        void calculateCameraEvent(CameraTiltEvent& cameraEvent, float durationSeconds, bool updateKinetic);
        void calculateCameraEvent(CameraZoomEvent& cameraEvent, float durationSeconds, bool updateKinetic);
    
        void moveToFitBounds(const MapBounds& mapBounds, const ScreenBounds& screenBounds, bool integerZoom, bool resetTilt, bool resetRotation, float durationSeconds);
        
        void onSurfaceCreated();
        void onSurfaceChanged(int width, int height);
        void onDrawFrame();
        void onSurfaceDestroyed();

        void finishRendering();

        void clearAndBindScreenFBO(const Color& color, bool depth, bool stencil);
        void blendAndUnbindScreenFBO(float opacity);
        void setZBuffering(bool enable);
    
        void calculateRayIntersectedElements(const MapPos& targetPos, ViewState& viewState, std::vector<RayIntersectedElement>& results);
    
        void billboardsChanged();
        void vtLabelsChanged(const std::shared_ptr<Layer>& layer, bool delay);
        void layerChanged(const std::shared_ptr<Layer>& layer, bool delay);
        void viewChanged(bool delay);
    
        void registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener);
        void unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener);
        
    private:
        class OptionsListener : public Options::OnChangeListener {
        public:
            explicit OptionsListener(const std::shared_ptr<MapRenderer>& mapRenderer);
            
            virtual void onOptionChanged(const std::string& optionName);
            
        private:
            std::weak_ptr<MapRenderer> _mapRenderer;
        };

        void initializeRenderState() const;

        void drawLayers(float deltaSeconds, const ViewState& viewState);
        
        void handleRendererCaptureCallbacks();

        static const int BILLBOARD_PLACEMENT_TASK_DELAY;
        static const int VT_LABEL_PLACEMENT_TASK_DELAY;

        static const std::string BLEND_VERTEX_SHADER;
        static const std::string BLEND_FRAGMENT_SHADER;
        
        std::chrono::steady_clock::time_point _lastFrameTime;
    
        ViewState _viewState;

        std::shared_ptr<GLResourceManager> _glResourceManager;

        std::shared_ptr<CullWorker> _cullWorker;
        std::thread _cullThread;
        
        std::shared_ptr<VTLabelPlacementWorker> _vtLabelPlacementWorker;
        std::thread _vtLabelPlacementThread;
        
        std::shared_ptr<OptionsListener> _optionsListener;

        std::vector<std::pair<GLuint, GLuint> > _screenBoundFBOs;
        std::map<GLuint, std::shared_ptr<FrameBuffer> > _screenFrameBuffers;
        std::shared_ptr<Shader> _screenBlendShader;
        
        BackgroundRenderer _backgroundRenderer;
        WatermarkRenderer _watermarkRenderer;
        
        std::vector<std::shared_ptr<BillboardDrawData> > _billboardDrawDatas;
        std::vector<std::shared_ptr<BillboardDrawData> > _billboardDrawDataBuffer;
        std::shared_ptr<BillboardPlacementWorker> _billboardPlacementWorker;
        std::thread _billboardPlacementThread;
    
        AnimationHandler _animationHandler;
        KineticEventHandler _kineticEventHandler;
        
        const std::shared_ptr<Layers> _layers;
        const std::shared_ptr<Options> _options;
        
        mutable std::atomic<bool> _surfaceCreated;
        mutable std::atomic<bool> _surfaceChanged;
        mutable std::atomic<bool> _billboardsChanged;
        mutable std::atomic<bool> _redrawPending;

        ThreadSafeDirectorPtr<RedrawRequestListener> _redrawRequestListener;

        ThreadSafeDirectorPtr<MapRendererListener> _mapRendererListener;

        std::vector<std::pair<DirectorPtr<RendererCaptureListener>, bool> > _rendererCaptureListeners;
        mutable std::mutex _rendererCaptureListenersMutex;

        std::vector<std::shared_ptr<OnChangeListener> > _onChangeListeners;
        mutable std::mutex _onChangeListenersMutex;

        mutable std::recursive_mutex _mutex;
    };
    
}

#endif
