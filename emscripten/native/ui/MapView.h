#ifndef _CARTO_EMSCRIPTENMAPVIEW_H_
#define _CARTO_EMSCRIPTENMAPVIEW_H_

#include <memory>
#include <string>

namespace carto {
    class BaseMapView;
    class RedrawRequestListener;
    class Layers;
    class MapBounds;
    class MapPos;
    class MapVec;
    class ScreenPos;
    class ScreenBounds;
    class MapEventListener;
    class MapRenderer;
    class Options;

    class MapView {
    public:
        MapView(std::string canvasId, bool runOnMainThread = false, bool stencil = true);
        ~MapView();

        static bool registerLicense(std::string licenseKey);

        void onSurfaceCreated();
        void onSurfaceChanged();
        void onDrawFrame();
        void requestRender();
        void start();
        bool isRenderPaused();
        void setRenderPaused(bool isPaused);
        bool getStencil();

        void onInputEvent(int event, float x1, float y1, float x2, float y2);
        void onWheelEvent(int delta, float x, float y);
        float getScale();
        int getCanvasWidth();
        int getCanvasHeight();
        std::string getCanvasId();
        bool needRedraw();
        std::shared_ptr<BaseMapView> getBaseMapView();

        const std::shared_ptr<Layers>& getLayers() const;
        const std::shared_ptr<Options>& getOptions() const;
        const std::shared_ptr<MapRenderer>& getMapRenderer() const;
        MapPos getFocusPos() const;
        float getRotation() const;
        float getTilt() const;
        float getZoom() const;
        void pan(MapVec deltaPos, float durationSeconds);
        void setFocusPos(MapPos pos, float durationSeconds);
        void rotate(float deltaAngle, float durationSeconds);
        void rotate(float deltaAngle, MapPos targetPos, float durationSeconds);
        void setRotation(float angle, float durationSeconds);
        void setRotation(float angle, MapPos targetPos, float durationSeconds);
        void tilt(float deltaTilt, float durationSeconds);
        void setTilt(float tilt, float durationSeconds);
        void zoom(float deltaZoom, float durationSeconds);
        void zoom(float deltaZoom, MapPos targetPos, float durationSeconds);
        void setZoom(float zoom, float durationSeconds);
        void setZoom(float zoom, MapPos targetPos, float durationSeconds);
        void moveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, bool integerZoom, float durationSeconds);
        void moveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, bool integerZoom, bool resetRotation, 
            bool resetTilt, float durationSeconds);
        std::shared_ptr<MapEventListener> getMapEventListener() const;
        void setMapEventListener(const std::shared_ptr<MapEventListener>& mapEventListener);
        MapPos screenToMap(ScreenPos screenPos);
        ScreenPos mapToScreen(MapPos mapPos);
        void cancelAllTasks();
        void clearPreloadingCaches();
        void clearAllCaches();

        static bool getRunOnMainThread();

    private:
        static void setRunOnMainThread(bool status);
        
        std::shared_ptr<BaseMapView> _baseMapView;
        std::shared_ptr<RedrawRequestListener> _redrawRequestListener;

        std::string _canvasId;
        float _scale = 1;
        std::atomic<bool> _needRedraw = false;

        int _canvasWidth = 0;
        int _canvasHeight = 0;
        bool _stencil = true;

        bool isPaused = false;
    };
}

#endif